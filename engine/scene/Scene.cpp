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
		for (auto& object : _objects)
		{
			object->update(deltaTime);
		}
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

	Camera& Scene::createCamera(glm::vec3 position, float fov, float aspect, float near, float far)
	{
		_camera = std::make_unique<Camera>(position, fov, aspect, near, far);
		return *_camera;
	}
}