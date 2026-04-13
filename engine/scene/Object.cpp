#include "scene/Object.h"

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
}