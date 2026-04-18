#include "resources/AssetManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <stb_image.h>
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

	Handle<Texture> AssetManager::loadTexture(const std::string& name, const std::string& path, bool alpha)
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

	Texture* AssetManager::getTexture(Handle<Texture> handle) const
	{
		assert(handle.index < _textures.assets.size() && "Texture index out of range: " + handle.index);
		return _textures.assets[handle.index].get();
	}

	Texture* AssetManager::getTexture(const std::string& name) const
	{
		return getTexture(_textures.nameToHandle.at(name));
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

	Material* AssetManager::getMaterial(Handle<Material> handle) const
	{
		assert(handle.index < _materials.assets.size() && "Material index out of range: " + handle.index);
		return _materials.assets[handle.index].get();
	}

	Material* AssetManager::getMaterial(const std::string& name) const
	{
		return getMaterial(_materials.nameToHandle.at(name));
	}
}