#include "scene/components/CharacterController.h"

#include <iostream>

#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"



namespace engine
{
	void CharacterController::start()
	{
		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		_currentSyncedWorldPosition = owner->transform.getWorldPosition();
		_previousSyncedWorldPosition = _currentSyncedWorldPosition;
		
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
		float stepHeight = 0.15f;
		_controller = new btKinematicCharacterController(_ghostObject, _shape.get(), stepHeight);
		_controller->setUp(btVector3(0, 1, 0));
		_controller->setGravity(btVector3(0, -gravity, 0));

		// Add to world with filters
		int group = btBroadphaseProxy::CharacterFilter;
		int mask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter;

		physics->addCollisionObject(_ghostObject, group, mask);
		physics->addAction(_controller);
	}

	void CharacterController::fitToMesh(const engine::Mesh& mesh)
	{
		auto bbox = mesh.getBBox();

		float modelHeight = bbox.max.y - bbox.min.y;
		float modelWidth = bbox.max.x - bbox.min.x;

		if (modelHeight > 0.0001f)
		{
			float scale = targetHeight / modelHeight;

			owner->transform.setScale(glm::vec3(scale));

			height = targetHeight;
			radius = modelWidth * scale * radiusScale;

			if (visualTransform)
			{
				float localOffsetY = (-height * 0.5f / scale) - bbox.min.y;
				visualTransform->setPosition(glm::vec3(0.0f, localOffsetY, 0.0f));
			}
		}
	}

	void CharacterController::update(float deltaTime)
	{
		_controller->setWalkDirection(PhysicsSystem::toBullet(_walkDirection));

		_walkDirection = glm::vec3(0.0f);
	}

	void CharacterController::postPhysicsUpdate(float deltaTime)
	{
		if (!_ghostObject || !_controller)
		{
			return;
		}

		if (_justTeleported)
		{
			_justTeleported = false;
			_previousSyncedWorldPosition = owner->transform.getPosition();
			_currentSyncedWorldPosition = _previousSyncedWorldPosition;
			_postPhysicsSyncedWorldPosition = _currentSyncedWorldPosition;
			return;
		}

		btTransform t = _ghostObject->getWorldTransform();
		_previousSyncedWorldPosition = _currentSyncedWorldPosition;
		const glm::vec3 syncedPosition = PhysicsSystem::toGlm(t.getOrigin());
		owner->transform.setPosition(syncedPosition);
		_currentSyncedWorldPosition = syncedPosition;
		_postPhysicsSyncedWorldPosition = _currentSyncedWorldPosition;
	}

	void CharacterController::moveWorldOffset(const glm::vec3& delta)
	{
		if (!_ghostObject || !_controller || glm::dot(delta, delta) <= 0.0f)
		{
			return;
		}

		const glm::vec3 newPosition = owner->transform.getPosition() + delta;
		btVector3 bulletPos = PhysicsSystem::toBullet(newPosition);

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(bulletPos);

		_controller->warp(bulletPos);
		_ghostObject->setWorldTransform(transform);
		_ghostObject->setInterpolationWorldTransform(transform);
		owner->transform.setPosition(newPosition);
		_previousSyncedWorldPosition = newPosition;
		_currentSyncedWorldPosition = newPosition;
	}

	void CharacterController::move(glm::vec3 direction)
	{
		_walkDirection = direction;
		if (_controller)
		{
			_controller->setWalkDirection(PhysicsSystem::toBullet(_walkDirection));
		}
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

	

void CharacterController::teleport(const glm::vec3& position)
{
	if (!_ghostObject || !_controller)
		return;

	glm::vec3 adjustedPos = position;
	adjustedPos.y += (height * 0.5f) + 0.25f;

	btVector3 bulletPos = PhysicsSystem::toBullet(adjustedPos);

	// Warp character controller
	_controller->warp(bulletPos);

	// Hard sync ghost transform
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(bulletPos);

	_ghostObject->setWorldTransform(transform);
	_ghostObject->setInterpolationWorldTransform(transform);

	// Clear velocities/motion
	_controller->setWalkDirection(btVector3(0, 0, 0));
	_controller->setVelocityForTimeInterval(btVector3(0, 0, 0), 0.0f);

	_walkDirection = glm::vec3(0.0f);
	owner->transform.setPosition(adjustedPos);

	// Flag to skip physics sync for one frame to let it settle
	_justTeleported = true;
}

		
}