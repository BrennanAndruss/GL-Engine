#pragma once

#include <glm/glm.hpp>
#include "scene/components/Component.h"

namespace engine
{
	class CharacterController : public Component
	{
	public:
		float height = 2.0f;
		float radius = 0.5f;
		float slopeLimit = 45.0f;

		void move(glm::vec3 delta);
		void update(float deltaTime) override;

		glm::vec3 getVelocity() const { return _velocity; }
		// bool getIsGrounded() const { return isGrounded; }

	private:
		glm::vec3 _velocity = glm::vec3(0.0f);
		glm::vec3 _targetVelocity = glm::vec3(0.0f);
		bool isGrounded = false;

		void resolveCollisions();
		// bool checkGrounded();
	};
}