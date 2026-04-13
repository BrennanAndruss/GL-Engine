#pragma once

#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <glm/glm.hpp>
#include "scene/Camera.h"
#include "scene/Object.h"
#include "scene/Light.h"

namespace engine
{
	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		void start();
		void update(float deltaTime);

		Camera& createCamera(glm::vec3 position, float fov, float aspect, float near = 0.1f, float far = 100.0f);
		Camera* getCamera() const { return _camera.get(); }

		void reserveObjects(size_t count) { _objects.reserve(count); }
		Object& createObject(const std::string& name);
		const std::vector<std::unique_ptr<Object>>& getObjects() const { return _objects; }

		template<typename T, typename... Args>
		T& createLight(Args&&... args)
		{
			static_assert(std::is_base_of_v<Light, T>, "T must derive from Light.");
			auto& ptr = _lights.emplace_back(
				std::make_unique<T>(std::forward<Args>(args)...)
			);
			return static_cast<T&>(*ptr);
		}
		const std::vector<std::unique_ptr<Light>>& getLights() const { return _lights; }

	private:
		std::unique_ptr<Camera> _camera;
		std::vector<std::unique_ptr<Object>> _objects;
		std::vector<std::unique_ptr<Light>> _lights;
	};
}