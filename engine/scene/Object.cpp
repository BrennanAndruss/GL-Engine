#include "scene/Object.h"

#include "resources/AssetManager.h"
#include "scene/components/Components.h"

namespace engine
{
	Object::Object(const std::string& name) : name(name) {}

	void Object::start()
	{
		_started = true;
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

	void Object::postPhysicsUpdate(float deltaTime)
	{
		for (auto& component : _components)
		{
			if (dynamic_cast<CharacterController*>(component.get()))
			{
				component->postPhysicsUpdate(deltaTime);
			}
		}

		for (auto& component : _components)
		{
			if (!dynamic_cast<CharacterController*>(component.get()))
			{
				component->postPhysicsUpdate(deltaTime);
			}
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