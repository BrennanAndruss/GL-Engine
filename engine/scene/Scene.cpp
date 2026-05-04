#include "scene/Scene.h"

#include "resources/AssetManager.h"

namespace engine
{
	void Scene::start()
	{
		// Resolve hierarchical transforms from scene initialization
		for (auto* root : getRootObjects())
		{
			resolveTransforms(root->transform, glm::mat4(1.0f));
		}

		// Start components
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

		if (_physics)
		{
			_physics->update(deltaTime);
		}

		// Resolve all dirty transforms recursively once per frame
		for (auto* root : getRootObjects())
		{
			resolveTransforms(root->transform, glm::mat4(1.0f));
		}

		// Sync camera with clean transforms
		if (_mainCamera) _mainCamera->updateViewMatrix();
	}

	void Scene::update(float deltaTime, AssetManager& assets)
	{
		for (auto& object : _objects)
		{
			object->update(deltaTime, assets);
		}

		if (_physics)
		{
			_physics->update(deltaTime);
		}

		for (auto* root : getRootObjects())
		{
			resolveTransforms(root->transform, glm::mat4(1.0f));
		}

		if (_mainCamera) _mainCamera->updateViewMatrix();
	}

	void Scene::resolveTransforms(Transform& t, const glm::mat4& parentWorld)
	{
		if (t.isDirty())
		{
			t.cleanWorldMatrix(parentWorld);
		}

		// Pass the clean world matrix down to children
		for (auto* child : t.getChildren())
		{
			resolveTransforms(*child, t.getWorldMatrix());
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
}