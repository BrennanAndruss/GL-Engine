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
#include "resources/Handle.h"

namespace engine
{
	class Cubemap;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		void start();
		void update(float deltaTime);

		Object& createObject(const std::string& name);

		std::vector<std::unique_ptr<Object>>& getObjects() { return _objects; }
		
		const std::vector<std::unique_ptr<Object>>& getObjects() const { return _objects; }
		std::vector<Object*> getRootObjects() const;

		void setPhysicsSystem(PhysicsSystem* physics) { _physics = physics; }
		PhysicsSystem* getPhysicsSystem() const { return _physics; }

		void setMainCamera(Camera* camera) { _mainCamera = camera; }
		Camera* getMainCamera() const { return _mainCamera; }

		void addLight(Light* light) { _lights.push_back(light); }
		const std::vector<Light*>& getLights() const { return _lights; }

		void setSkybox(Handle<Cubemap> cubemap)
		{
			_skybox = cubemap;
			_hasSkybox = cubemap.valid();
		}

		Handle<Cubemap> getSkybox() const { return _skybox; }
		bool hasSkybox() const { return _hasSkybox; }

	private:
		std::vector<std::unique_ptr<Object>> _objects;
		bool _started = false;

		PhysicsSystem* _physics = nullptr;
		Camera* _mainCamera = nullptr;
		std::vector<Light*> _lights;

		Handle<Cubemap> _skybox;
		bool _hasSkybox = false;

		void resolveTransforms(Transform& t, const glm::mat4& parentWorld);
	};
}