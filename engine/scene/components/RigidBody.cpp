#include "scene/components/RigidBody.h"

#include <btBulletDynamicsCommon.h>
#include <cassert>
#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"

namespace engine
{
	void RigidBody::start()
	{
		_collider = owner->getComponent<Collider>();
		assert(_collider && "RigidBody requires Collider component");

		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		_body = physics->createBody(
			_collider->getShape(), owner->transform.getWorldPosition(), 
			mass, _collider->isTrigger
		);

		// Set user pointer for callbacks
		_body->setUserPointer(owner);
	}

	void RigidBody::update(float deltaTime)
	{
		btTransform t;
		_body->getMotionState()->getWorldTransform(t);

		// Sync the engine transform with the physics world transform
		owner->transform.setPosition(PhysicsSystem::toGlm(t.getOrigin()));
		owner->transform.setRotation(PhysicsSystem::toGlm(t.getRotation()));
	}
}