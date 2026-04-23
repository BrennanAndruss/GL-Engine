#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <memory>
#include "resources/Handle.h"

// Forward declarations of resource types
namespace engine
{
	class Shader;
	class Texture;
	class Mesh;
	struct Material;
	class Heightmap;
}

namespace engine
{
	template<typename T>
	struct AssetPool
	{
		std::vector<std::unique_ptr<T>> assets;
		std::unordered_map<std::string, Handle<T>> nameToHandle;
	};

	class AssetManager
	{
	public:
		AssetManager(const std::string& assetDir);
		~AssetManager();

		// Loading
		Handle<Shader> loadShader(const std::string& name, 
								  const std::string& vertPath, 
								  const std::string& fragPath);

		Handle<Texture> loadTexture(const std::string& name, 
									const std::string& path, 
									bool alpha);

		Handle<Mesh> loadMesh(const std::string& name, const std::string& path);
		Handle<Material> loadMaterial(const std::string& name);

		Handle<Heightmap> loadHeightmap(const std::string& name, 
										const std::string& path, 
										float heightScale);

		// Generation 
		Handle<Mesh> createPlaneMesh(const std::string& name, 
									 int planeRes, 
									 float planeLen);

		Handle<Mesh> createHeightmapMesh(const std::string& name, 
										 Handle<Heightmap> heightmapHandle,
										 int planeRes,
										 float planeLen);
		
		Shader* getShader(Handle<Shader> handle) const;
		Shader* getShader(const std::string& name) const;
		Handle<Shader> getShaderHandle(const std::string& name) const;

		Texture* getTexture(Handle<Texture> handle) const;
		Texture* getTexture(const std::string& name) const;
		Handle<Texture> getTextureHandle(const std::string& name) const;

		Mesh* getMesh(Handle<Mesh> handle) const;
		Mesh* getMesh(const std::string& name) const;
		Handle<Mesh> getMeshHandle(const std::string& name) const;

		Material* getMaterial(Handle<Material> handle) const;
		Material* getMaterial(const std::string& name) const;
		Handle<Material> getMaterialHandle(const std::string& name) const;

		Heightmap* getHeightmap(Handle<Heightmap> handle) const;
		Heightmap* getHeightmap(const std::string& name) const;

	private:
		AssetPool<Shader> _shaders;
		AssetPool<Texture> _textures;
		AssetPool<Mesh> _meshes;
		AssetPool<Material> _materials;
		AssetPool<Heightmap> _heightmaps;

		std::filesystem::path _assetDir;
		std::filesystem::path resolvePath(const std::string& rel) const;
	};
}