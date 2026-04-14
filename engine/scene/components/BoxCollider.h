#pragma once

#include <glm/glm.hpp>
#include <physics/PhysicsSystem.h>
#include <scene/components/Component.h>

namespace engine
{
	class BoxCollider : public Component
	{
	public:
		bool isTrigger = false;
		glm::vec3 center = glm::vec3(0.0f);
		glm::vec3 size = glm::vec3(1.0f);

	private:
		PhysicsSystem* _physics = nullptr;
	};
}