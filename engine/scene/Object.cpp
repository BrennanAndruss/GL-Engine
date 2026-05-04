#include "scene/Object.h"

#include "resources/AssetManager.h"

namespace engine
{
	Object::Object(const std::string& name) : name(name) {}

	void Object::start()
	{
		for (auto& component : _components)
		{
			component->start();
		}
	}

	void Object::update(float deltaTime)
	{
		for (auto& component : _components)
		{
			component->update(deltaTime);
		}
	}

	void Object::update(float deltaTime, AssetManager& assets)
	{
		for (auto& component : _components)
		{
			component->update(deltaTime, assets);
		}
	}
}