#include "resources/AssetManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <array>
#include <unordered_map>
#include <cmath>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "resources/Heightmap.h"
#include "resources/SkeletalAnimation.h"
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

	static glm::mat4 aiToGlm(const aiMatrix4x4& m)
	{
		glm::mat4 result(1.0f);
		result[0][0] = m.a1; result[1][0] = m.a2; result[2][0] = m.a3; result[3][0] = m.a4;
		result[0][1] = m.b1; result[1][1] = m.b2; result[2][1] = m.b3; result[3][1] = m.b4;
		result[0][2] = m.c1; result[1][2] = m.c2; result[2][2] = m.c3; result[3][2] = m.c4;
		result[0][3] = m.d1; result[1][3] = m.d2; result[2][3] = m.d3; result[3][3] = m.d4;
		return result;
	}

	static void addBoneInfluence(glm::uvec4& ids, glm::vec4& weights, unsigned int boneId, float weight)
	{
		for (std::size_t i = 0; i < MAX_BONE_INFLUENCES; ++i)
		{
			if (weights[i] == 0.0f)
			{
				ids[i] = boneId;
				weights[i] = weight;
				return;
			}
		}

		std::size_t smallestIdx = 0;
		for (std::size_t i = 1; i < MAX_BONE_INFLUENCES; ++i)
		{
			if (weights[i] < weights[smallestIdx])
			{
				smallestIdx = i;
			}
		}

		if (weight > weights[smallestIdx])
		{
			ids[smallestIdx] = boneId;
			weights[smallestIdx] = weight;
		}
	}

	static std::unique_ptr<Mesh> buildMeshFromAiMesh(const aiMesh* aiMeshData)
	{
		std::vector<glm::vec3> positions(aiMeshData->mNumVertices);
		std::vector<glm::vec3> normals(aiMeshData->mNumVertices, glm::vec3(0.0f, 1.0f, 0.0f));
		std::vector<glm::vec2> texcoords(aiMeshData->mNumVertices, glm::vec2(0.0f));
		std::vector<glm::uvec4> boneIds(aiMeshData->mNumVertices, glm::uvec4(0));
		std::vector<glm::vec4> boneWeights(aiMeshData->mNumVertices, glm::vec4(0.0f));
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

		if (!aiMeshData->HasBones() || aiMeshData->mNumBones == 0)
		{
			return std::make_unique<Mesh>(positions, normals, texcoords, indices);
		}

		std::unordered_map<std::string, unsigned int> boneNameToId;
		boneNameToId.reserve(aiMeshData->mNumBones);

		unsigned int nextBoneId = 0;
		for (unsigned int boneIndex = 0; boneIndex < aiMeshData->mNumBones; ++boneIndex)
		{
			const aiBone* bone = aiMeshData->mBones[boneIndex];
			if (!bone) continue;

			const std::string boneName = bone->mName.C_Str();
			auto [it, inserted] = boneNameToId.emplace(boneName, nextBoneId);
			if (inserted)
			{
				++nextBoneId;
			}

			const unsigned int mappedBoneId = it->second;
			for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
			{
				const aiVertexWeight& vertexWeight = bone->mWeights[weightIndex];
				if (vertexWeight.mVertexId >= aiMeshData->mNumVertices)
				{
					continue;
				}

				addBoneInfluence(
					boneIds[vertexWeight.mVertexId],
					boneWeights[vertexWeight.mVertexId],
					mappedBoneId,
					vertexWeight.mWeight
				);
			}
		}

		for (glm::vec4& weights : boneWeights)
		{
			const float sum = weights.x + weights.y + weights.z + weights.w;
			if (sum > 0.0f)
			{
				weights /= sum;
			}
		}

		return std::make_unique<Mesh>(positions, normals, texcoords, indices, boneIds, boneWeights);
	}

	static void buildSkeletonNodeTree(const aiNode* node, int parentIndex, Skeleton& skeleton)
	{
		const int nodeIndex = static_cast<int>(skeleton.nodes.size());
		skeleton.nodes.push_back({
			node->mName.C_Str(),
			parentIndex,
			{},
			aiToGlm(node->mTransformation),
			glm::mat4(1.0f),
			false,
			-1
		});
		skeleton.nodeLookup[skeleton.nodes.back().name] = nodeIndex;

		if (parentIndex >= 0)
		{
			skeleton.nodes[parentIndex].children.push_back(nodeIndex);
		}
		else
		{
			skeleton.rootNodeIndex = nodeIndex;
		}

		for (unsigned int childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
		{
			buildSkeletonNodeTree(node->mChildren[childIndex], nodeIndex, skeleton);
		}
	}

	static std::unique_ptr<Skeleton> buildSkeletonFromAiMesh(const aiScene* scene, const aiMesh* aiMeshData)
	{
		auto skeleton = std::make_unique<Skeleton>();
		skeleton->globalInverseTransform = glm::inverse(aiToGlm(scene->mRootNode->mTransformation));
		buildSkeletonNodeTree(scene->mRootNode, -1, *skeleton);

		for (unsigned int boneIndex = 0; boneIndex < aiMeshData->mNumBones; ++boneIndex)
		{
			const aiBone* bone = aiMeshData->mBones[boneIndex];
			if (!bone) continue;

			const std::string boneName = bone->mName.C_Str();
			const int nodeIndex = skeleton->findNodeIndex(boneName);
			if (nodeIndex < 0)
			{
				continue;
			}

			SkeletonNode& node = skeleton->nodes[nodeIndex];
			node.isBone = true;
			node.boneIndex = static_cast<int>(boneIndex);
			node.inverseBindMatrix = aiToGlm(bone->mOffsetMatrix);
		}

		return skeleton;
	}

	static std::unique_ptr<AnimationClip> buildAnimationClipFromAiAnimation(const aiAnimation* aiAnimationData)
	{
		auto clip = std::make_unique<AnimationClip>();
		clip->name = aiAnimationData->mName.C_Str();
		clip->durationTicks = static_cast<float>(aiAnimationData->mDuration);
		clip->ticksPerSecond = (aiAnimationData->mTicksPerSecond > 0.0) ? static_cast<float>(aiAnimationData->mTicksPerSecond) : 25.0f;
		clip->tracks.reserve(aiAnimationData->mNumChannels);

		for (unsigned int channelIndex = 0; channelIndex < aiAnimationData->mNumChannels; ++channelIndex)
		{
			const aiNodeAnim* channel = aiAnimationData->mChannels[channelIndex];
			if (!channel)
			{
				continue;
			}

			BoneTrack track;
			track.nodeName = channel->mNodeName.C_Str();
			track.positions.reserve(channel->mNumPositionKeys);
			track.rotations.reserve(channel->mNumRotationKeys);
			track.scales.reserve(channel->mNumScalingKeys);

			for (unsigned int keyIndex = 0; keyIndex < channel->mNumPositionKeys; ++keyIndex)
			{
				const auto& key = channel->mPositionKeys[keyIndex];
				track.positions.push_back({
					static_cast<float>(key.mTime),
					glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z)
				});
			}

			for (unsigned int keyIndex = 0; keyIndex < channel->mNumRotationKeys; ++keyIndex)
			{
				const auto& key = channel->mRotationKeys[keyIndex];
				track.rotations.push_back({
					static_cast<float>(key.mTime),
					glm::normalize(glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z))
				});
			}

			for (unsigned int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex)
			{
				const auto& key = channel->mScalingKeys[keyIndex];
				track.scales.push_back({
					static_cast<float>(key.mTime),
					glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z)
				});
			}

			clip->trackLookup[track.nodeName] = clip->tracks.size();
			clip->tracks.push_back(std::move(track));
		}

		return clip;
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

	Handle<Skeleton> AssetManager::loadSkeletonAssimp(const std::string& name, const std::string& path,
		unsigned int meshIndex)
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
			throw std::runtime_error("Failed to load skeleton with Assimp: " + absPath +
				"\nassimp reason: " + importer.GetErrorString());
		}

		if (meshIndex >= scene->mNumMeshes)
		{
			throw std::runtime_error("Assimp mesh index out of range for skeleton import: " + absPath);
		}

		const aiMesh* aiMeshData = scene->mMeshes[meshIndex];
		if (!aiMeshData->HasBones())
		{
			throw std::runtime_error("Assimp mesh has no bones: " + absPath);
		}

		_skeletons.assets.emplace_back(buildSkeletonFromAiMesh(scene, aiMeshData));
		Handle<Skeleton> handle = { _skeletons.assets.size() - 1 };
		_skeletons.nameToHandle[name] = handle;
		return handle;
	}

	Handle<AnimationClip> AssetManager::loadAnimationClipAssimp(const std::string& name, const std::string& path,
		unsigned int animationIndex)
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

		if (!scene || !scene->HasAnimations())
		{
			throw std::runtime_error("Failed to load animation clip with Assimp: " + absPath +
				"\nassimp reason: " + importer.GetErrorString());
		}

		if (animationIndex >= scene->mNumAnimations)
		{
			throw std::runtime_error("Assimp animation index out of range for: " + absPath);
		}

		_animationClips.assets.emplace_back(buildAnimationClipFromAiAnimation(scene->mAnimations[animationIndex]));
		Handle<AnimationClip> handle = { _animationClips.assets.size() - 1 };
		_animationClips.nameToHandle[name] = handle;
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

	Skeleton* AssetManager::getSkeleton(Handle<Skeleton> handle) const
	{
		if (!handle.valid() || handle.index >= _skeletons.assets.size()) return nullptr;
		return _skeletons.assets[handle.index].get();
	}

	Skeleton* AssetManager::getSkeleton(const std::string& name) const
	{
		auto it = _skeletons.nameToHandle.find(name);
		if (it == _skeletons.nameToHandle.end()) return nullptr;
		return getSkeleton(it->second);
	}

	Handle<Skeleton> AssetManager::getSkeletonHandle(const std::string& name) const
	{
		auto it = _skeletons.nameToHandle.find(name);
		if (it == _skeletons.nameToHandle.end()) return {};
		return it->second;
	}

	AnimationClip* AssetManager::getAnimationClip(Handle<AnimationClip> handle) const
	{
		if (!handle.valid() || handle.index >= _animationClips.assets.size()) return nullptr;
		return _animationClips.assets[handle.index].get();
	}

	AnimationClip* AssetManager::getAnimationClip(const std::string& name) const
	{
		auto it = _animationClips.nameToHandle.find(name);
		if (it == _animationClips.nameToHandle.end()) return nullptr;
		return getAnimationClip(it->second);
	}

	Handle<AnimationClip> AssetManager::getAnimationClipHandle(const std::string& name) const
	{
		auto it = _animationClips.nameToHandle.find(name);
		if (it == _animationClips.nameToHandle.end()) return {};
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