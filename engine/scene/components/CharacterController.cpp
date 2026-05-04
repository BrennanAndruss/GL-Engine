#include "scene/components/CharacterController.h"

#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"

namespace engine
{
	void CharacterController::start()
	{
		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		
		_shape = std::make_unique<btCapsuleShape>(radius, height - (2.0f * radius));

		_ghostObject = new btPairCachingGhostObject();
		_ghostObject->setCollisionShape(_shape.get());

		btTransform t;
		t.setIdentity();
		t.setOrigin(PhysicsSystem::toBullet(owner->transform.getWorldPosition()));
		_ghostObject->setWorldTransform(t);

		// Set user pointer for callbacks
		_ghostObject->setUserPointer(owner);
		_ghostObject->setCollisionFlags(_ghostObject->getCollisionFlags() |
			btCollisionObject::CF_CHARACTER_OBJECT);
		_ghostObject->setActivationState(DISABLE_DEACTIVATION);

		// Configure controller logic
		float stepHeight = 0.45f;
		_controller = new btKinematicCharacterController(_ghostObject, _shape.get(), stepHeight);
		_controller->setUp(btVector3(0, 1, 0));
		_controller->setGravity(btVector3(0, -9.81f, 0));

		// Add to world with filters
		int group = btBroadphaseProxy::CharacterFilter;
		int mask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter;

		physics->addCollisionObject(_ghostObject, group, mask);
		physics->addAction(_controller);
	}

	void CharacterController::update(float deltaTime)
	{
		_controller->setWalkDirection(PhysicsSystem::toBullet(_walkDirection));

		// Sync the engine transform with the physics world transform
		btTransform t = _ghostObject->getWorldTransform();
		owner->transform.setPosition(PhysicsSystem::toGlm(t.getOrigin()));
		owner->transform.setRotation(PhysicsSystem::toGlm(t.getRotation()));

		_walkDirection = glm::vec3(0.0f);
	}

	void CharacterController::move(glm::vec3 direction)
	{
		_walkDirection = direction;
	}

	bool CharacterController::isOnGround() const
	{
		return _controller ? _controller->onGround() : false;
	}

	bool CharacterController::canJump() const
	{
		return _controller ? _controller->canJump() : false;
	}

	void CharacterController::jump(glm::vec3 direction)
	{
		if (_controller)
		{
			_controller->jump(btVector3(direction.x, direction.y, direction.z));
		}
	}
}