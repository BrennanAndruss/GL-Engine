#include "scene/components/SphereCollider.h"

#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/components/RigidBody.h"

namespace engine
{
	void SphereCollider::start()
	{
		PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
		_shape = std::make_unique<btSphereShape>(radius);

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