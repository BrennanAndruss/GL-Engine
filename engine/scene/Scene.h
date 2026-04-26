#pragma once

#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <glm/glm.hpp>
#include "physics/PhysicsSystem.h"
#include "scene/Camera.h"
#include "scene/Object.h"
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

		void setPhysicsSystem(PhysicsSystem* physics) { _physics = physics; }
		PhysicsSystem* getPhysicsSystem() const { return _physics; }

		Camera& createCamera(glm::vec3 position, float fov, float aspect, 
			float near = 0.1f, float far = 100.0f);
		Camera* getCamera() const { return _camera.get(); }

		Object& createObject(const std::string& name);

		// All objects for flat traversal
		const std::vector<std::unique_ptr<Object>>& getObjects() const { return _objects; }

		// Root objects for hierarchical traversal
		std::vector<Object*> getRootObjects() const;

		void addLight(Light* light) { _lights.push_back(light); }
		const std::vector<Light*>& getLights() const { return _lights; }

	private:
		PhysicsSystem* _physics = nullptr;

		std::vector<std::unique_ptr<Object>> _objects;

		std::unique_ptr<Camera> _camera;
		std::vector<Light*> _lights;
	};
}