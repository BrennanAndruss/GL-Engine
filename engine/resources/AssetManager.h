#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <memory>

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
		std::unordered_map<std::string, size_t> nameToId;
	};

	class AssetManager
	{
	public:
		AssetManager(const std::string& assetDir);
		~AssetManager();

		size_t loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
		size_t loadTexture(const std::string& name, const std::string& path, bool alpha);
		size_t loadMesh(const std::string& name, const std::string& path);
		size_t loadMaterial(const std::string& name);

		Shader* getShader(size_t id) const;
		Shader* getShader(const std::string& name) const;

		Texture* getTexture(size_t id) const;
		Texture* getTexture(const std::string& name) const;

		Mesh* getMesh(size_t id) const;
		Mesh* getMesh(const std::string& name) const;

		Material* getMaterial(size_t id) const;
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