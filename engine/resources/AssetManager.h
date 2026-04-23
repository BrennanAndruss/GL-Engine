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

		Handle<Shader> loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
		Handle<Texture> loadTexture(const std::string& name, const std::string& path, bool alpha);
		Handle<Mesh> loadMesh(const std::string& name, const std::string& path);
		Handle<Mesh> createPlaneMesh(const std::string& name, int planeRes, float planeLen);
		Handle<Material> loadMaterial(const std::string& name);

		Shader* getShader(Handle<Shader> id) const;
		Shader* getShader(const std::string& name) const;

		Texture* getTexture(Handle<Texture> id) const;
		Texture* getTexture(const std::string& name) const;

		Mesh* getMesh(Handle<Mesh> id) const;
		Mesh* getMesh(const std::string& name) const;

		Material* getMaterial(Handle<Material> id) const;
		Material* getMaterial(const std::string& name) const;

	private:
		AssetPool<Shader> _shaders;
		AssetPool<Texture> _textures;
		AssetPool<Mesh> _meshes;
		AssetPool<Material> _materials;

		std::filesystem::path _assetDir;
		std::filesystem::path resolvePath(const std::string& rel) const;
	};
}