#include "scene/components/RigidBody.h"

#include <btBulletDynamicsCommon.h>
#include <cassert>
#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"

namespace engine
{
	RigidBody::~RigidBody()
	{
		destroyBody();
	}

	void RigidBody::destroyBody()
	{
		if (!_body) return;

		if (auto* physics = owner->getScene()->getPhysicsSystem())
		{
			// Unregister callback and remove from physics world
			physics->removeBody(_body);
		}

		delete _body;
		_body = nullptr;
	}

	void RigidBody::disablePhysics()
	{
		_physicsDisabled = true;
		destroyBody();
	}

	bool RigidBody::initializeBody()
	{
		if (_physicsDisabled) return false;
		if (_body && !_bodyDirty) return true;

		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		if (!physics) return false;

		if (_body)
		{
			destroyBody();
		}

		if (!_collider)
		{
			_collider = owner->getComponent<Collider>();
		}
		if (!_collider) return false;

		_collider->destroyCollisionObject();

		const float effectiveMass = bodyType == BodyType::Dynamic ? mass : 0.0f;
		_body = physics->createBody(
			_collider->getShape(), owner->transform.getWorldPosition(), 
			effectiveMass, _collider->isTrigger
		);

		if (bodyType == BodyType::Kinematic)
		{
			_body->setCollisionFlags(
				_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT
			);
			_body->setActivationState(DISABLE_DEACTIVATION);
		}

		// Set user pointer for callbacks
		_body->setUserPointer(owner);
		_bodyDirty = false;
		return true;
	}

	void RigidBody::setBodyType(BodyType type)
	{
		if (bodyType == type) return;

		bodyType = type;
		_bodyDirty = true;
		initializeBody();
	}

	void RigidBody::start()
	{
		initializeBody();
	}

	void RigidBody::update(float deltaTime)
	{
		if (_physicsDisabled || !_body) return;

		if (bodyType == BodyType::Static || bodyType == BodyType::Kinematic)
		{
			btTransform t;
			t.setIdentity();
			t.setOrigin(PhysicsSystem::toBullet(owner->transform.getWorldPosition()));
			t.setRotation(PhysicsSystem::toBullet(owner->transform.getWorldRotation()));
			_body->setWorldTransform(t);
			if (auto* motionState = _body->getMotionState())
			{
				motionState->setWorldTransform(t);
			}
			if (bodyType == BodyType::Kinematic)
			{
				_body->setActivationState(DISABLE_DEACTIVATION);
			}
			return;
		}

		btTransform t;
		_body->getMotionState()->getWorldTransform(t);

		// Sync the engine transform with the physics world transform
		owner->transform.setPosition(PhysicsSystem::toGlm(t.getOrigin()));
		owner->transform.setRotation(PhysicsSystem::toGlm(t.getRotation()));
	}

	void RigidBody::setLinearVelocity(glm::vec3 velocity)
	{
		if (!_body) return;
		_body->setLinearVelocity(PhysicsSystem::toBullet(velocity));
	}

	glm::vec3 RigidBody::getLinearVelocity() const
	{
		if (!_body) return glm::vec3(0.0f);
		return PhysicsSystem::toGlm(_body->getLinearVelocity());
	}
}