#include "scene/Scene.h"

namespace engine
{
	void Scene::start()
	{
		for (auto& object : _objects)
		{
			object->start();
		}
	}

	void Scene::update(float deltaTime)
	{
		// Run all component logic updates
		for (auto& object : _objects)
		{
			object->update(deltaTime);
		}

		// Step physics
		_physics->update(deltaTime);

		// todo: Resolve all dirty transforms recursively
		// (currently handled in transform class as needed)
		// ...

		// Sync camera with clean transforms
		if (_mainCamera) _mainCamera->updateViewMatrix();
	}

	Object& Scene::createObject(const std::string& name)
	{
		auto& object = _objects.emplace_back(std::make_unique<Object>(name));
		object->setScene(this);
		return *object;
	}

	std::vector<Object*> Scene::getRootObjects() const
	{
		std::vector<Object*> roots;
		for (const auto& object : _objects)
		{
			if (!object->transform.getParent())
			{
				roots.push_back(object.get());
			}
		}
		return roots;
	}
}