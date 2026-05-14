#include "systems/Collectable.h"

#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/components/RigidBody.h"
#include "scene/components/MeshRenderer.h"
#include "scene/components/Collider.h"
#include "systems/PlayerController.h"
#include "MyGame.h"

#include <iostream>

namespace
{
	btCollisionObject* getCollisionObjectForCollectable(engine::Object* owner)
	{
		auto* collider = owner->getComponent<engine::Collider>();
		if (collider && collider->getCollisionObject())
		{
			return collider->getCollisionObject();
		}

		auto* rigidBody = owner->getComponent<engine::RigidBody>();
		if (rigidBody && rigidBody->getBody())
		{
			return rigidBody->getBody();
		}

		return nullptr;
	}
}

void Collectable::tryRegisterCallback()
{
	if (_callbackRegistered || !owner)
	{
		return;
	}

	auto* collisionObject = getCollisionObjectForCollectable(owner);
	if (!collisionObject)
	{
		return;
	}

	auto* physics = owner->getScene()->getPhysicsSystem();
	if (!physics)
	{
		return;
	}

	physics->registerCallback(collisionObject, [this](btCollisionObject* other)
		{
			// The callback is attached to the collectable, so only accept the player object.
			auto* otherOwner = other ? static_cast<engine::Object*>(other->getUserPointer()) : nullptr;
			if (!otherOwner || !otherOwner->getComponent<PlayerController>())
			{
				return;
			}

			onCollected();
		});
	_callbackRegistered = true;
}

void Collectable::start()
{
	tryRegisterCallback();
}

void Collectable::update(float deltaTime)
{
	(void)deltaTime;
	tryRegisterCallback();
}

void Collectable::onCollected()
{
	if (isCollected)
	{
		return;
	}

	std::cout << "collected" << std::endl;
	isCollected = true;

	if (auto* game = MyGame::getActiveGame())
	{
		game->onCollectableCollected();
	}

	// Clean up physics before deletion
	if (auto* physics = owner->getScene()->getPhysicsSystem())
	{
		if (auto* rigidBody = owner->getComponent<engine::RigidBody>())
		{
			rigidBody->disablePhysics();
		}

		if (auto* collider = owner->getComponent<engine::Collider>())
		{
			auto* collisionObject = collider->releaseCollisionObject();
			if (collisionObject)
			{
				physics->removeCollisionObject(collisionObject);
			}
		}
	}

	// Delete the object from the scene
	auto& objects = owner->getScene()->getObjects();
	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		if (it->get() == owner)
		{
			objects.erase(it);
			break;
		}
	}
}