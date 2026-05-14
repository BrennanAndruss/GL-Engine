#include "scene/components/Collider.h"

#include <btBulletDynamicsCommon.h>
#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/components/RigidBody.h"

namespace engine
{
	void Collider::update(float deltaTime)
	{
		if (!_object)
		{
			return;
		}

		btTransform t;
		t.setIdentity();
		t.setOrigin(PhysicsSystem::toBullet(owner->transform.getWorldPosition()));
		t.setRotation(PhysicsSystem::toBullet(owner->transform.getWorldRotation()));
		_object->setWorldTransform(t);
	}

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
		const glm::vec3 worldCenter = center * worldScale;

		// If object doesn't have a RigidBody, register as static collision object
		if (!owner->getComponent<RigidBody>())
		{
			_object = physics->createCollisionObject(
				_shape.get(), owner->transform.getWorldPosition() + worldCenter, isTrigger);
			_object->setUserPointer(owner);

			// Keep trigger objects active
			if (isTrigger)
			{
				_object->setActivationState(DISABLE_DEACTIVATION);
			}
		}
	}

	void BoxCollider::update(float deltaTime)
	{
		if (_shape)
		{
			const glm::vec3 worldScale = owner->transform.getWorldScale();
			_shape->setLocalScaling(PhysicsSystem::toBullet(worldScale));
			if (_object)
			{
				btTransform t;
				t.setIdentity();
				t.setOrigin(PhysicsSystem::toBullet(owner->transform.getWorldPosition() + (center * worldScale)));
				t.setRotation(PhysicsSystem::toBullet(owner->transform.getWorldRotation()));
				_object->setWorldTransform(t);
			}
		}
	}

	void BoxCollider::rebuild()
	{
		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		
		// Recreate the shape with the new size
		_shape = std::make_unique<btBoxShape>(PhysicsSystem::toBullet(size));
		
		glm::vec3 worldScale = owner->transform.getWorldScale();
		_shape->setLocalScaling(PhysicsSystem::toBullet(worldScale));
		const glm::vec3 worldCenter = center * worldScale;
		
		// Remove the old collision object from physics system
		if (_object)
		{
			physics->removeCollisionObject(_object);
			_object = nullptr;
		}
		
		// Create new collision object if no RigidBody
		if (!owner->getComponent<RigidBody>())
		{
			_object = physics->createCollisionObject(
				_shape.get(), owner->transform.getWorldPosition() + worldCenter, isTrigger);
			_object->setUserPointer(owner);
			
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
		const glm::vec3 worldCenter = center * worldScale;

		// If object doesn't have a RigidBody, register as static collision object
		if (!owner->getComponent<RigidBody>())
		{
			_object = physics->createCollisionObject(
				_shape.get(), owner->transform.getWorldPosition() + worldCenter, isTrigger);
			_object->setUserPointer(owner);

			// Keep trigger objects active
			if (isTrigger)
			{
				_object->setActivationState(DISABLE_DEACTIVATION);
			}
		}
	}

	void SphereCollider::update(float deltaTime)
	{
		if (_shape)
		{
			const glm::vec3 worldScale = owner->transform.getWorldScale();
			_shape->setLocalScaling(PhysicsSystem::toBullet(worldScale));
			if (_object)
			{
				btTransform t;
				t.setIdentity();
				t.setOrigin(PhysicsSystem::toBullet(owner->transform.getWorldPosition() + (center * worldScale)));
				t.setRotation(PhysicsSystem::toBullet(owner->transform.getWorldRotation()));
				_object->setWorldTransform(t);
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
		const glm::vec3 worldCenter = center * worldScale;

		// If object doesn't have a RigidBody, register as static collision object
		if (!owner->getComponent<RigidBody>())
		{
			_object = physics->createCollisionObject(
				_shape.get(), owner->transform.getWorldPosition() + worldCenter, isTrigger);
			_object->setUserPointer(owner);

			// Keep trigger objects active
			if (isTrigger)
			{
				_object->setActivationState(DISABLE_DEACTIVATION);
			}
		}
	}

	void CapsuleCollider::update(float deltaTime)
	{
		if (_shape)
		{
			const glm::vec3 worldScale = owner->transform.getWorldScale();
			_shape->setLocalScaling(PhysicsSystem::toBullet(worldScale));
			if (_object)
			{
				btTransform t;
				t.setIdentity();
				t.setOrigin(PhysicsSystem::toBullet(owner->transform.getWorldPosition() + (center * worldScale)));
				t.setRotation(PhysicsSystem::toBullet(owner->transform.getWorldRotation()));
				_object->setWorldTransform(t);
			}
		}
	}
}