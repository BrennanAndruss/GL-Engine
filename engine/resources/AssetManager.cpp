#include "resources/AssetManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <array>
#include <stb_image.h>
#include <glm/glm.hpp>
#include "resources/Heightmap.h"
#include "renderer/resources/Shader.h"
#include "renderer/resources/Texture.h"
#include "renderer/resources/Cubemap.h"
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

		_shaders.assets.emplace_back(std::make_unique<Shader>(vertSrc, fragSrc));
		Handle<Shader> handle = { _shaders.assets.size() - 1 };
		_shaders.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Texture> AssetManager::loadTexture(const std::string& name,
		const std::string& path, bool alpha)
	{
		std::string absPath = resolvePath(path).string();

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
		catch (const std::runtime_error&)
		{
			stbi_image_free(textureData);
			throw std::runtime_error("Failed to initialize texture: " + absPath +
				"\nstb reason: " + stbi_failure_reason());
		}

		std::size_t id = _textures.assets.size();
		texture->setUnit(static_cast<GLint>(id));

		Handle<Texture> handle = { id };
		_textures.assets.push_back(std::move(texture));
		_textures.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Cubemap> AssetManager::loadCubemap(const std::string& name,
		const std::array<std::string, 6>& facePaths)
	{
		std::array<std::string, 6> absPaths;
		for (std::size_t i = 0; i < 6; ++i)
		{
			absPaths[i] = resolvePath(facePaths[i]).string();
		}

		auto cubemap = std::make_unique<Cubemap>(absPaths);

		std::size_t id = _cubemaps.assets.size();
		cubemap->setUnit(static_cast<GLint>(id));

		Handle<Cubemap> handle = { id };
		_cubemaps.assets.push_back(std::move(cubemap));
		_cubemaps.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Mesh> AssetManager::loadMesh(const std::string& name, const std::string& path)
	{
		std::string absPath = resolvePath(path).string();

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> objMaterials;
		std::string errStr;
		bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, absPath.c_str());

		if (!rc)
		{
			throw std::runtime_error("Failed to load mesh: " + errStr);
		}

		_meshes.assets.emplace_back(std::make_unique<Mesh>(shapes[0]));
		Handle<Mesh> handle = { _meshes.assets.size() - 1 };
		_meshes.nameToHandle[name] = handle;

		return handle;
	}

	Handle<Heightmap> AssetManager::loadHeightmap(const std::string& name,
		const std::string& path, float heightScale)
	{
		std::string absPath = resolvePath(path).string();

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

		std::vector<glm::vec3> positions((planeRes + 1) * (planeRes + 1));
		std::vector<glm::vec3> normals((planeRes + 1) * (planeRes + 1), glm::vec3(0.0f, 1.0f, 0.0f));
		std::vector<glm::vec2> texcoords((planeRes + 1) * (planeRes + 1));

		for (int i = 0, x = 0; x <= planeRes; ++x)
		{
			for (int z = 0; z <= planeRes; ++z, ++i)
			{
				positions[i] = glm::vec3(
					x * stepLen - halfLen,
					0.0f,
					z * stepLen - halfLen
				);

				texcoords[i] = glm::vec2(
					static_cast<float>(x) / planeRes,
					static_cast<float>(z) / planeRes
				);
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

		std::vector<glm::vec3> positions((planeRes + 1) * (planeRes + 1));
		std::vector<glm::vec3> normals((planeRes + 1) * (planeRes + 1), glm::vec3(0.0f));
		std::vector<glm::vec2> texcoords((planeRes + 1) * (planeRes + 1));

		for (int i = 0, x = 0; x <= planeRes; ++x)
		{
			for (int z = 0; z <= planeRes; ++z, ++i)
			{
				float u = static_cast<float>(x) / planeRes;
				float v = static_cast<float>(z) / planeRes;
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

		for (std::size_t i = 0; i < indices.size(); i += 3)
		{
			unsigned int i0 = indices[i + 0];
			unsigned int i1 = indices[i + 1];
			unsigned int i2 = indices[i + 2];

			glm::vec3 edge1 = positions[i1] - positions[i0];
			glm::vec3 edge2 = positions[i2] - positions[i0];
			glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

			normals[i0] += faceNormal;
			normals[i1] += faceNormal;
			normals[i2] += faceNormal;
		}

		for (glm::vec3& n : normals)
		{
			n = glm::normalize(n);
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
		if (!handle.valid() || handle.index >= _shaders.assets.size()) return nullptr;
		return _shaders.assets[handle.index].get();
	}

	Shader* AssetManager::getShader(const std::string& name) const
	{
		auto it = _shaders.nameToHandle.find(name);
		if (it == _shaders.nameToHandle.end()) return nullptr;
		return getShader(it->second);
	}

	Handle<Shader> AssetManager::getShaderHandle(const std::string& name) const
	{
		auto it = _shaders.nameToHandle.find(name);
		if (it == _shaders.nameToHandle.end()) return {};
		return it->second;
	}

	Texture* AssetManager::getTexture(Handle<Texture> handle) const
	{
		if (!handle.valid() || handle.index >= _textures.assets.size()) return nullptr;
		return _textures.assets[handle.index].get();
	}

	Texture* AssetManager::getTexture(const std::string& name) const
	{
		auto it = _textures.nameToHandle.find(name);
		if (it == _textures.nameToHandle.end()) return nullptr;
		return getTexture(it->second);
	}

	Handle<Texture> AssetManager::getTextureHandle(const std::string& name) const
	{
		auto it = _textures.nameToHandle.find(name);
		if (it == _textures.nameToHandle.end()) return {};
		return it->second;
	}

	Cubemap* AssetManager::getCubemap(Handle<Cubemap> handle) const
	{
		if (!handle.valid() || handle.index >= _cubemaps.assets.size()) return nullptr;
		return _cubemaps.assets[handle.index].get();
	}

	Cubemap* AssetManager::getCubemap(const std::string& name) const
	{
		auto it = _cubemaps.nameToHandle.find(name);
		if (it == _cubemaps.nameToHandle.end()) return nullptr;
		return getCubemap(it->second);
	}

	Handle<Cubemap> AssetManager::getCubemapHandle(const std::string& name) const
	{
		auto it = _cubemaps.nameToHandle.find(name);
		if (it == _cubemaps.nameToHandle.end()) return {};
		return it->second;
	}

	Mesh* AssetManager::getMesh(Handle<Mesh> handle) const
	{
		if (!handle.valid() || handle.index >= _meshes.assets.size()) return nullptr;
		return _meshes.assets[handle.index].get();
	}

	Mesh* AssetManager::getMesh(const std::string& name) const
	{
		auto it = _meshes.nameToHandle.find(name);
		if (it == _meshes.nameToHandle.end()) return nullptr;
		return getMesh(it->second);
	}

	Handle<Mesh> AssetManager::getMeshHandle(const std::string& name) const
	{
		auto it = _meshes.nameToHandle.find(name);
		if (it == _meshes.nameToHandle.end()) return {};
		return it->second;
	}

	Material* AssetManager::getMaterial(Handle<Material> handle) const
	{
		if (!handle.valid() || handle.index >= _materials.assets.size()) return nullptr;
		return _materials.assets[handle.index].get();
	}

	Material* AssetManager::getMaterial(const std::string& name) const
	{
		auto it = _materials.nameToHandle.find(name);
		if (it == _materials.nameToHandle.end()) return nullptr;
		return getMaterial(it->second);
	}

	Handle<Material> AssetManager::getMaterialHandle(const std::string& name) const
	{
		auto it = _materials.nameToHandle.find(name);
		if (it == _materials.nameToHandle.end()) return {};
		return it->second;
	}

	Heightmap* AssetManager::getHeightmap(Handle<Heightmap> handle) const
	{
		if (!handle.valid() || handle.index >= _heightmaps.assets.size()) return nullptr;
		return _heightmaps.assets[handle.index].get();
	}

	Heightmap* AssetManager::getHeightmap(const std::string& name) const
	{
		auto it = _heightmaps.nameToHandle.find(name);
		if (it == _heightmaps.nameToHandle.end()) return nullptr;
		return getHeightmap(it->second);
	}
}