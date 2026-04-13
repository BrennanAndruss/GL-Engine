#include "scene/components/CharacterController.h"

#include "scene/Object.h"

namespace engine
{
	void CharacterController::move(glm::vec3 delta)
	{
		_velocity.x = delta.x;
		_velocity.z = delta.z;
	}

	void CharacterController::update(float deltaTime)
	{
		// Integrate position
		owner->transform.translation += _velocity * deltaTime;
	}

	void CharacterController::resolveCollisions()
	{

	}
}