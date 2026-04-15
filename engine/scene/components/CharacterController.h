#pragma once

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <glm/glm.hpp>
#include <memory>
#include "scene/components/Component.h"

namespace engine
{
	class CharacterController : public Component
	{
	public:
		float height = 2.0f;
		float radius = 0.5f;

		void start() override;
		void update(float deltaTime) override;

		void move(glm::vec3 delta);

	private:
		btPairCachingGhostObject* _ghostObject = nullptr;
		btKinematicCharacterController* _controller = nullptr;
		std::unique_ptr<btCapsuleShape> _shape;

		glm::vec3 _walkDirection = glm::vec3(0.0f);
	};
}