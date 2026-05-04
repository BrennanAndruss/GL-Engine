#pragma once

#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <glm/glm.hpp>
#include "physics/PhysicsSystem.h"
#include "scene/Object.h"
#include "scene/components/Camera.h"
#include "scene/components/Light.h"

namespace engine
{
	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		void start();
		void update(float deltaTime);

		Object& createObject(const std::string& name);

		// All objects for flat traversal
		const std::vector<std::unique_ptr<Object>>& getObjects() const { return _objects; }

		// Root objects for hierarchical traversal
		std::vector<Object*> getRootObjects() const;

		void setPhysicsSystem(PhysicsSystem* physics) { _physics = physics; }
		PhysicsSystem* getPhysicsSystem() const { return _physics; }

		void setMainCamera(Camera* camera) { _mainCamera = camera; }
		Camera* getMainCamera() const { return _mainCamera; }

		void addLight(Light* light) { _lights.push_back(light); }
		const std::vector<Light*>& getLights() const { return _lights; }

	private:
		std::vector<std::unique_ptr<Object>> _objects;
		bool _started = false;

		PhysicsSystem* _physics = nullptr;
		Camera* _mainCamera = nullptr;
		std::vector<Light*> _lights;

		void resolveTransforms(Transform& t, const glm::mat4& parentWorld);
	};
}