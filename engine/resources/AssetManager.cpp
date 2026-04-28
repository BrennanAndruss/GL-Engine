#include "resources/AssetManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "resources/Heightmap.h"
#include "renderer/resources/Shader.h"
#include "renderer/resources/Texture.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"

namespace engine
{
	static std::string readFile(const std::filesystem::path& path)
	{
		std::ifstream fileHandle(path.string());
		if (!fileHandle)
		{
			throw std::runtime_error("Could not open file: " + path.string());
		}

		std::stringstream ss;
		ss << fileHandle.rdbuf();
		return ss.str();
	}

	static std::unique_ptr<Mesh> buildMeshFromAiMesh(const aiMesh* aiMeshData)
	{
		std::vector<glm::vec3> positions(aiMeshData->mNumVertices);
		std::vector<glm::vec3> normals(aiMeshData->mNumVertices, glm::vec3(0.0f, 1.0f, 0.0f));
		std::vector<glm::vec2> texcoords(aiMeshData->mNumVertices, glm::vec2(0.0f));
		std::vector<unsigned int> indices;

		for (unsigned int i = 0; i < aiMeshData->mNumVertices; ++i)
		{
			//convert from Assimp's aiVector3D to glm::vec3
			positions[i] = glm::vec3(
				aiMeshData->mVertices[i].x,
				aiMeshData->mVertices[i].y,
				aiMeshData->mVertices[i].z
			);

			if (aiMeshData->HasNormals())
			{
				normals[i] = glm::vec3(
					aiMeshData->mNormals[i].x,
					aiMeshData->mNormals[i].y,
					aiMeshData->mNormals[i].z
				);
			}

			if (aiMeshData->HasTextureCoords(0))
			{
				texcoords[i] = glm::vec2(
					aiMeshData->mTextureCoords[0][i].x, 
					aiMeshData->mTextureCoords[0][i].y
				);
			}
		}
        
		indices.reserve(aiMeshData->mNumFaces * 3); //preallocate memory for the indices (assuming triangles)
		for (unsigned int i = 0; i < aiMeshData->mNumFaces; ++i)
		{
			const aiFace& face = aiMeshData->mFaces[i]; 
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);  
			}
		}

		return std::make_unique<Mesh>(positions, normals, texcoords, indices);
	}

	AssetManager::AssetManager(const std::string& assetDir) : _assetDir(assetDir) {}

	AssetManager::~AssetManager() = default;

	std::filesystem::path AssetManager::resolvePath(const std::string& rel) const
	{
		return _assetDir / rel;
	}

	Handle<Shader> AssetManager::loadShader(const std::string& name,
		const std::string& vertPath, const std::string& fragPath)
	{
		std::filesystem::path absVertPath = resolvePath(vertPath);
		std::string vertSrc = readFile(absVertPath);

		std::filesystem::path absFragPath = resolvePath(fragPath);
		std::string fragSrc = readFile(absFragPath);

		// Create and store asset in shader pool
		_shaders.assets.emplace_back(std::make_unique<Shader>(vertSrc, fragSrc));
		Handle<Shader> handle = { _shaders.assets.size() - 1 };
		_shaders.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Texture> AssetManager::loadTexture(const std::string& name, 
		const std::string& path, bool alpha)
	{
		std::string absPath = resolvePath(path).string();

		// Load image data
		int width, height, nrChannels;
		unsigned char* textureData = stbi_load(absPath.c_str(), &width, &height, &nrChannels, 0);
		if (!textureData)
		{
			throw std::runtime_error("Failed to load texture: " + absPath);
		}

		std::unique_ptr<Texture> texture;
		try
		{
			GLint format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
			texture = std::make_unique<Texture>(width, height, format, textureData);
			stbi_image_free(textureData);
		}
		catch (const std::runtime_error& e)
		{
			stbi_image_free(textureData);
			throw std::runtime_error("Failed to initialize texture: " + absPath +
				"\nstb reason: " + stbi_failure_reason());
		}

		// Set unique texture unit
		std::size_t id = _textures.assets.size();
		texture->setUnit(static_cast<GLint>(id));

		// Store asset in texture pool
		Handle<Texture> handle = { id };
		_textures.assets.push_back(std::move(texture));
		_textures.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Mesh> AssetManager::loadMesh(const std::string& name, const std::string& path)
	{
		// Keep the existing public API, but route loading through the Assimp path.
		// This avoids tight coupling to the legacy tinyobj loader here and supports
		// additional formats (fbx, gltf, glb, ...) through one code path.
		return loadMeshAssimp(name, path, 0);
	}

	Handle<Mesh> AssetManager::loadMeshAssimp(const std::string& name,
		const std::string& path, unsigned int meshIndex)
	{
		const std::string absPath = resolvePath(path).string();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			absPath,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs
		);

		if (!scene || !scene->HasMeshes())
		{
			throw std::runtime_error("Failed to load mesh with Assimp: " + absPath +
				"\nassimp reason: " + importer.GetErrorString());
		}

		if (meshIndex >= scene->mNumMeshes)
		{
			throw std::runtime_error("Assimp mesh index out of range for: " + absPath);
		}

		_meshes.assets.emplace_back(buildMeshFromAiMesh(scene->mMeshes[meshIndex]));
		Handle<Mesh> handle = { _meshes.assets.size() - 1 };
		_meshes.nameToHandle[name] = handle;

		return handle;
	}

	std::vector<Handle<Mesh>> AssetManager::loadModelMeshesAssimp(const std::string& namePrefix,
		const std::string& path)
	{
		const std::string absPath = resolvePath(path).string();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			absPath,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs
		);

		if (!scene || !scene->HasMeshes())
		{
			throw std::runtime_error("Failed to load model meshes with Assimp: " + absPath +
				"\nassimp reason: " + importer.GetErrorString());
		}

		std::vector<Handle<Mesh>> handles;
		handles.reserve(scene->mNumMeshes);

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			const std::string meshName = namePrefix + "_" + std::to_string(i);
			_meshes.assets.emplace_back(buildMeshFromAiMesh(scene->mMeshes[i]));

			Handle<Mesh> handle = { _meshes.assets.size() - 1 };
			_meshes.nameToHandle[meshName] = handle;
			handles.push_back(handle);
		}

		return handles;
	}

	Handle<Heightmap> AssetManager::loadHeightmap(const std::string& name,
		const std::string& path, float heightScale)
	{
		std::string absPath = resolvePath(path).string();

		// Load image data
		int width, height, nrChannels;
		unsigned char* data = stbi_load(absPath.c_str(), &width, &height, &nrChannels, 0);
		if (!data)
		{
			throw std::runtime_error("Failed to load texture: " + absPath);
		}

		_heightmaps.assets.emplace_back(std::make_unique<Heightmap>(
			width, height, nrChannels, data, heightScale
		));
		stbi_image_free(data);

		Handle<Heightmap> handle = { _heightmaps.assets.size() - 1 };
		_heightmaps.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Mesh> AssetManager::createPlaneMesh(const std::string& name, int planeRes, float planeLen)
	{
		float halfLen = planeLen * 0.5f;
		float stepLen = planeLen / static_cast<float>(planeRes);
		// Resize Buffers to fit resolution-- how many vertices / positions.
		std::vector<glm::vec3> positions((planeRes + 1) * (planeRes + 1));
		std::vector<glm::vec3> normals((planeRes + 1) * (planeRes + 1), glm::vec3(0.0f, 1.0f, 0.0f));
		std::vector<glm::vec2> texcoords((planeRes + 1) * (planeRes + 1));

		for (int i = 0, x = 0; x <= planeRes; ++x)
		{
			for (int z = 0; z <= planeRes; ++z, ++i)
			{
				positions[i] = glm::vec3(
					x * stepLen - halfLen,
					0.0f, //here is where we would sample the y from heightmap.png
					z * stepLen - halfLen
				);

				texcoords[i] = glm::vec2(
					static_cast<float>(x) / planeRes,
					static_cast<float>(z) / planeRes
				);
			}
		}

		std::vector<unsigned int> indices(planeRes * planeRes * 6); // triangles buffer generated 

		for (int ti = 0, vi = 0, x = 0; x < planeRes; ++x, ++vi)
		{
			for (int z = 0; z < planeRes; ++z, ti += 6, ++vi)
			{
				indices[ti + 0] = vi;
				indices[ti + 1] = vi + 1;
				indices[ti + 2] = vi + planeRes + 1;

				indices[ti + 3] = vi + planeRes + 1;
				indices[ti + 4] = vi + 1;
				indices[ti + 5] = vi + planeRes + 2;
			}
		}

		_meshes.assets.emplace_back(std::make_unique<Mesh>(positions, normals, texcoords, indices));
		Handle<Mesh> handle = { _meshes.assets.size() - 1 };
		_meshes.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Mesh> AssetManager::createHeightmapMesh(const std::string& name,
												   Handle<Heightmap> heightmapHandle,
												   int planeRes,
												   float planeLen)
	{
		Heightmap heightmap = *_heightmaps.assets[heightmapHandle.index].get();

		float halfLen = planeLen * 0.5f;
		float stepLen = planeLen / static_cast<float>(planeRes);
		// init buffers to hold data
		std::vector<glm::vec3> positions((planeRes + 1) * (planeRes + 1));
		std::vector<glm::vec3> normals((planeRes + 1) * (planeRes + 1), glm::vec3(0.0f));
		std::vector<glm::vec2> texcoords((planeRes + 1) * (planeRes + 1));

		for (int i = 0, x = 0; x <= planeRes; ++x)
		{
			for (int z = 0; z <= planeRes; ++z, ++i)
			{
				float u = static_cast<float>(x) / planeRes;
				float v = static_cast<float>(z) / planeRes;
				// === MAIN DIFFERENCE BETWEEN PLANE & TERRAINPLANE ===
				float height = heightmap.sample(u, v) * heightmap.getHeightScale();

				positions[i] = glm::vec3(
					x * stepLen - halfLen,
					height,
					z * stepLen - halfLen
				);

				texcoords[i] = glm::vec2(u, v);
			}
		}

		std::vector<unsigned int> indices(planeRes * planeRes * 6);

		for (int ti = 0, vi = 0, x = 0; x < planeRes; ++x, ++vi)
		{
			for (int z = 0; z < planeRes; ++z, ti += 6, ++vi)
			{
				indices[ti + 0] = vi;
				indices[ti + 1] = vi + 1;
				indices[ti + 2] = vi + planeRes + 1;

				indices[ti + 3] = vi + planeRes + 1;
				indices[ti + 4] = vi + 1;
				indices[ti + 5] = vi + planeRes + 2;
			}
		}

		// Compute smooth vertex normals from triangle faces
		for (std::size_t i = 0; i < indices.size(); i += 3)
		{
			unsigned int i0 = indices[i + 0];
			unsigned int i1 = indices[i + 1];
			unsigned int i2 = indices[i + 2];

			glm::vec3 edge1 = positions[i1] - positions[i0];
			glm::vec3 edge2 = positions[i2] - positions[i0];
			// cross product to get normals
			glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

			normals[i0] += faceNormal;
			normals[i1] += faceNormal;
			normals[i2] += faceNormal;
		}
		
		// normalize normals 
		for (glm::vec3& normal : normals)
		{
			if (glm::length(normal) > 0.0f)
			{
				normal = glm::normalize(normal);
			}
			else
			{
				normal = glm::vec3(0.0f, 1.0f, 0.0f);
			}
		}

		_meshes.assets.emplace_back(std::make_unique<Mesh>(positions, normals, texcoords, indices));
		Handle<Mesh> handle = { _meshes.assets.size() - 1 };
		_meshes.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Material> AssetManager::loadMaterial(const std::string& name)
	{
		_materials.assets.emplace_back(std::make_unique<Material>());
		Handle<Material> handle = { _materials.assets.size() - 1 };
		_materials.nameToHandle[name] = handle;

		return handle;
	}

	Shader* AssetManager::getShader(Handle<Shader> handle) const
	{
		assert(handle.index < _shaders.assets.size() && "Shader index out of range: " + handle.index);
		return _shaders.assets[handle.index].get();
	}

	Shader* AssetManager::getShader(const std::string& name) const 
	{
		return getShader(_shaders.nameToHandle.at(name));
	}

	Handle<Shader> AssetManager::getShaderHandle(const std::string& name) const
	{
		return _shaders.nameToHandle.at(name);
	}

	Texture* AssetManager::getTexture(Handle<Texture> handle) const
	{
		assert(handle.index < _textures.assets.size() && "Texture index out of range: " + handle.index);
		return _textures.assets[handle.index].get();
	}

	Texture* AssetManager::getTexture(const std::string& name) const
	{
		return getTexture(_textures.nameToHandle.at(name));
	}

	Handle<Texture> AssetManager::getTextureHandle(const std::string& name) const
	{
		return _textures.nameToHandle.at(name);
	}

	Mesh* AssetManager::getMesh(Handle<Mesh> handle) const
	{
		assert(handle.index < _meshes.assets.size() && "Mesh index out of range: " + handle.index);
		return _meshes.assets[handle.index].get();
	}

	Mesh* AssetManager::getMesh(const std::string& name) const
	{
		return getMesh(_meshes.nameToHandle.at(name));
	}

	Handle<Mesh> AssetManager::getMeshHandle(const std::string& name) const
	{
		return _meshes.nameToHandle.at(name);
	}

	Material* AssetManager::getMaterial(Handle<Material> handle) const
	{
		assert(handle.index < _materials.assets.size() && "Material index out of range: " + handle.index);
		return _materials.assets[handle.index].get();
	}

	Material* AssetManager::getMaterial(const std::string& name) const
	{
		return getMaterial(_materials.nameToHandle.at(name));
	}

	Handle<Material> AssetManager::getMaterialHandle(const std::string& name) const
	{
		return _materials.nameToHandle.at(name);
	}
	
	Heightmap* AssetManager::getHeightmap(Handle<Heightmap> handle) const
	{
		assert(handle.index < _heightmaps.assets.size() && "Heightmap index out of range: " + handle.index);
		return _heightmaps.assets[handle.index].get();
	}

	Heightmap* AssetManager::getHeightmap(const std::string& name) const
	{
		return getHeightmap(_heightmaps.nameToHandle.at(name));
	}
}