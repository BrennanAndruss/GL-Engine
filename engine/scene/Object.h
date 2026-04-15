#pragma once

#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include "scene/Transform.h"
#include "scene/components/Component.h"

namespace engine
{
	class Scene;
}

namespace engine
{
	class Object
	{
	public:
		std::string name;
		Transform transform;

		Object() = default;
		Object(const std::string& name);
		~Object() = default;

		void start();
		void update(float deltaTime);

		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
			auto& ptr = _components.emplace_back(
				std::make_unique<T>(std::forward<Args>(args)...)
			);
			ptr->owner = this;
			return static_cast<T&>(*ptr);
		}

		template<typename T>
		T* getComponent() const
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
			for (auto& component : _components)
			{
				if (auto* c = dynamic_cast<T*>(component.get()))
					return c;
			}
			return nullptr;
		}

		void setScene(Scene* scene) { _scene = scene; }
		Scene* getScene() const { return _scene; }

	private:
		Scene* _scene = nullptr;

		std::vector<std::unique_ptr<Component>> _components;
	};
}