#include "scene/Scene.h"

#include <iostream>
#include "resources/AssetManager.h"

namespace engine
{
	void Scene::start()
	{
		_started = true;

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

		// Delete objects marked for deletion
		cleanupDeleted();

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

		cleanupDeleted();

		for (auto* root : getRootObjects())
		{
			resolveTransforms(root->transform, glm::mat4(1.0f));
		}

		if (_mainCamera) _mainCamera->updateViewMatrix();
	}

	void Scene::cleanupDeleted()
	{
		bool objectsToErase = false;
		for (auto& object : _objects)
		{
			if (object->markedForDeletion)
			{
				objectsToErase = true;

				// Unparent object and its children
				object->transform.setParent(nullptr);
				for (auto* child : object->transform.getChildren())
				{
					child->setParent(nullptr);
				}
			}
		}
		
		if (!objectsToErase) return;
		std::cout << "size before: " << _objects.size() << std::endl;
		_objects.erase(
			std::remove_if(_objects.begin(), _objects.end(),
				[](const std::unique_ptr<Object>& object)
				{
					return object->markedForDeletion;
				}),
			_objects.end()
		);
		std::cout << "size after: " << _objects.size() << std::endl;
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
		if (_started)
		{
			object->start();
		}
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