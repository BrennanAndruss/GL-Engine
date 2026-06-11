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
			if (isCollected)
			{
    			return;
			}
			// The callback is attached to the collectable, so only accept the player object.
			auto* otherOwner = other ? static_cast<engine::Object*>(other->getUserPointer()) : nullptr;
			if (!otherOwner || !otherOwner->getComponent<PlayerController>())
			{
				return;
			}

			if (auto* playerController = otherOwner->getComponent<PlayerController>())
			{
    			if (type == Type::speedBoost)
				{
    				playerController->activateSpeedBoost();

    				if (auto* game = MyGame::getActiveGame())
    				{
        				game->onPowerUpCollected(type, playerController->speedBoostDuration);
    				}
				}
				else if (type == Type::JumpBoost)
				{
    				playerController->activateJumpBoost();

    				if (auto* game = MyGame::getActiveGame())
    				{
        				game->onPowerUpCollected(type, playerController->jumpBoostDuration);
    				}
				}
    			else if (type == Type::Cyan || type == Type::Magenta || type == Type::Yellow)
    			{
        			playerController->triggerGemCelebrate();
    			}
			}

			onCollected();
		});
	_callbackRegistered = true;
}

void Collectable::start()
{
    originalScale = owner->transform.getScale();

    if (auto* meshRenderer = owner ? owner->getComponent<engine::MeshRenderer>() : nullptr)
    {
        meshRenderer->writeStencil = true;
        originalMesh = meshRenderer->mesh;
    }

    std::cout << "Collectable start: " << owner->name << std::endl;

    tryRegisterCallback();
}

void Collectable::update(float deltaTime)
{
    tryRegisterCallback();

    if (!isCollected)
    {
        return;
    }

    if (type != Type::speedBoost && type != Type::JumpBoost)
    {
        return;
    }

    respawnTimer -= deltaTime;

    if (respawnTimer <= 0.0f)
	{
    	respawnTimer = 0.0f;
    	isCollected = false;

    	if (owner)
    	{
        	owner->transform.setScale(originalScale);

        	if (auto* meshRenderer = owner->getComponent<engine::MeshRenderer>())
        	{
           		meshRenderer->mesh = originalMesh;
        	}
    	}
	}
}

void Collectable::onCollected()
{
    if (isCollected)
    {
        return;
    }

    isCollected = true;

    if (type == Type::speedBoost || type == Type::JumpBoost)
    {
        respawnDuration = 15.0f;
        respawnTimer = respawnDuration;

        if (owner)
		{
    		if (auto* meshRenderer = owner->getComponent<engine::MeshRenderer>())
    		{
        		meshRenderer->mesh = Handle<engine::Mesh>();
    		}
		}

        return;
    }

    if (auto* game = MyGame::getActiveGame())
    {
        if (type == Type::Cyan || type == Type::Magenta || type == Type::Yellow)
        {
            game->onCollectableCollected(type, owner->transform.getWorldPosition());
        }
    }

    owner->markedForDeletion = true;
}