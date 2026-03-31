#include "AssetManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include "../../external/stb_image/stb_image.h"
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

	AssetManager::AssetManager(const std::string& assetDir) : _assetDir(assetDir) {}

	AssetManager::~AssetManager() = default;

	std::filesystem::path AssetManager::resolvePath(const std::string& rel) const
	{
		return _assetDir / rel;
	}

	size_t AssetManager::loadShader(const std::string& name,
		const std::string& vertPath, const std::string& fragPath)
	{
		std::filesystem::path absVertPath = resolvePath(vertPath);
		std::string vertSrc = readFile(absVertPath);

		std::filesystem::path absFragPath = resolvePath(fragPath);
		std::string fragSrc = readFile(absFragPath);

		auto shader = std::make_unique<Shader>(vertSrc, fragSrc);

		// Store asset in shader pool
		size_t id = _shaders.assets.size();
		_shaders.assets.push_back(std::move(shader));
		_shaders.nameToId[name.data()] = id;

		return id;
	}

	size_t AssetManager::loadTexture(const std::string& name, const std::string& path, bool alpha)
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
		size_t id = _textures.assets.size();
		texture->setUnit(static_cast<GLint>(id));

		// Store asset in texture pool
		_textures.assets.push_back(std::move(texture));
		_textures.nameToId[name] = id;

		return id;
	}

	size_t AssetManager::loadMesh(const std::string& name, const std::string& path)
	{
		std::string absPath = resolvePath(path).string();

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> objMaterials;
		std::string errStr;
		bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, absPath.c_str());

		if (rc)
		{
			throw std::runtime_error("Failed to load mesh: " + errStr);
		}

		auto mesh = std::make_unique<Mesh>(shapes[0]);

		// Store asset in mesh pool
		size_t id = _meshes.assets.size();
		_meshes.assets.push_back(std::move(mesh));
		_meshes.nameToId[name] = id;

		return id;
	}

	size_t AssetManager::loadMaterial(const std::string& name)
	{
		auto material = std::make_unique<Material>();

		size_t id = _materials.assets.size();
		_materials.assets.push_back(std::move(material));
		_materials.nameToId[name.data()] = id;

		return id;
	}

	Shader* AssetManager::getShader(size_t id)
	{
		assert(id < _shaders.assets.size() && "Shader ID out of range: " + id);
		return _shaders.assets[id].get();
	}

	Shader* AssetManager::getShader(const std::string& name)
	{
		return getShader(_shaders.nameToId.at(name));
	}

	Texture* AssetManager::getTexture(size_t id)
	{
		assert(id < _textures.assets.size() && "Texture ID out of range: " + id);
		return _textures.assets[id].get();
	}

	Texture* AssetManager::getTexture(const std::string& name)
	{
		return getTexture(_textures.nameToId.at(name));
	}

	Mesh* AssetManager::getMesh(size_t id)
	{
		assert(id < _meshes.assets.size() && "Mesh ID out of range: " + id);
		return _meshes.assets[id].get();
	}

	Mesh* AssetManager::getMesh(const std::string& name)
	{
		return getMesh(_meshes.nameToId.at(name));
	}

	Material* AssetManager::getMaterial(size_t id)
	{
		assert(id < _materials.assets.size() && "Material ID out of range: " + id);
		return _materials.assets[id].get();
	}

	Material* AssetManager::getMaterial(const std::string& name)
	{
		return getMaterial(_materials.nameToId.at(name));
	}
}