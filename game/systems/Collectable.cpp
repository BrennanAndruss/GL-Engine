#include "systems/Collectable.h"

#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/components/MeshRenderer.h"
#include "scene/components/Collider.h"

void Collectable::start()
{
	auto* collider = owner->getComponent<engine::Collider>();
	if (collider && collider->getCollisionObject())
	{
		auto* physics = owner->getScene()->getPhysicsSystem();
		physics->registerCallback(collider->getCollisionObject(), [this](btCollisionObject* other)
			{
				this->onCollected();
			});
	}
}

void Collectable::onCollected()
{
	auto* meshRenderer = owner->getComponent<engine::MeshRenderer>();
	if (meshRenderer)
	{
		meshRenderer->materialId = collectedMatId;
		isCollected = true;
	}
}