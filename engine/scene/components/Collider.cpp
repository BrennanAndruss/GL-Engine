#include "scene/components/Collider.h"

#include <btBulletDynamicsCommon.h>
#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/components/RigidBody.h"

namespace engine
{
	// Constructors defined in implementation where classes are included
	BoxCollider::BoxCollider() = default;
	BoxCollider::~BoxCollider() = default;

	btCollisionShape* BoxCollider::getShape() const
	{
		return _shape.get();
	}

	void BoxCollider::start()
	{
		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		_shape = std::make_unique<btBoxShape>(PhysicsSystem::toBullet(size));

		glm::vec3 worldScale = owner->transform.getWorldScale();
		_shape->setLocalScaling(PhysicsSystem::toBullet(worldScale));

		// If object doesn't have a RigidBody, register as static collision object
		if (!owner->getComponent<RigidBody>())
		{
			_object = physics->createCollisionObject(
				_shape.get(), owner->transform.getWorldPosition(), isTrigger);

			// Keep trigger objects active
			if (isTrigger)
			{
				_object->setActivationState(DISABLE_DEACTIVATION);
			}
		}
	}

	SphereCollider::SphereCollider() = default;
	SphereCollider::~SphereCollider() = default;

	btCollisionShape* SphereCollider::getShape() const
	{
		return _shape.get();
	}

	void SphereCollider::start()
	{
		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		_shape = std::make_unique<btSphereShape>(radius);

		glm::vec3 worldScale = owner->transform.getWorldScale();
		_shape->setLocalScaling(PhysicsSystem::toBullet(worldScale));

		// If object doesn't have a RigidBody, register as static collision object
		if (!owner->getComponent<RigidBody>())
		{
			_object = physics->createCollisionObject(
				_shape.get(), owner->transform.getWorldPosition(), isTrigger);

			// Keep trigger objects active
			if (isTrigger)
			{
				_object->setActivationState(DISABLE_DEACTIVATION);
			}
		}
	}

	CapsuleCollider::CapsuleCollider() = default;
	CapsuleCollider::~CapsuleCollider() = default;

	btCollisionShape* CapsuleCollider::getShape() const
	{
		return _shape.get();
	}

	void CapsuleCollider::start()
	{
		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();

		// Convert to bullet's internal height (sphere center-to-center)
		float bulletHeight = height - (2.0f * radius);
		if (bulletHeight < 0) bulletHeight = 0.0f;

		switch (direction)
		{
		case Axis::X:
			_shape = std::make_unique<btCapsuleShapeX>(radius, bulletHeight);
			break;
		case Axis::Y:
			_shape = std::make_unique<btCapsuleShape>(radius, bulletHeight);
			break;
		case Axis::Z:
			_shape = std::make_unique<btCapsuleShapeZ>(radius, bulletHeight);
			break;
		}

		glm::vec3 worldScale = owner->transform.getWorldScale();
		_shape->setLocalScaling(PhysicsSystem::toBullet(worldScale));

		// If object doesn't have a RigidBody, register as static collision object
		if (!owner->getComponent<RigidBody>())
		{
			_object = physics->createCollisionObject(
				_shape.get(), owner->transform.getWorldPosition(), isTrigger);

			// Keep trigger objects active
			if (isTrigger)
			{
				_object->setActivationState(DISABLE_DEACTIVATION);
			}
		}
	}
}