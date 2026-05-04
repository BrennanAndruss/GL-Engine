#include "scene/components/HeightmapCollider.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <cassert>
#include "physics/PhysicsSystem.h"
#include "scene/Scene.h"
#include "scene/Object.h"

namespace engine
{
    HeightmapCollider::HeightmapCollider() = default;
    HeightmapCollider::~HeightmapCollider() = default;

    btCollisionShape* HeightmapCollider::getShape() const
    {
        return _shape.get();
    }

    void HeightmapCollider::start()
    {
        assert(heightmap && "HeightmapCollider requires height data");

        PhysicsSystem* physics = owner->getScene()->getPhysicsSystem();
        _shape = std::make_unique<btHeightfieldTerrainShape>(
            heightmap->getWidth(), heightmap->getLength(),
            heightmap->getHeightData().data(), 1.0f,
            0.0f, heightmap->getHeightScale(), 1, PHY_FLOAT, false
        );

        // Set local scaling to match terrain mesh resolution and size
        float planeRes = static_cast<float>(heightmap->getWidth() - 1);
        float doubleScale = planeLen / planeRes;
        _shape->setLocalScaling(btVector3(doubleScale, 1.0f, doubleScale));

        // Bullet heightmap terrain shape spans [-halfHeight, halfHeight]
        // Shift the collision object by halfHeight to align with the visual mesh
        float halfHeight = heightmap->getHeightScale() * 0.5f;
        glm::vec3 worldPos = owner->transform.getWorldPosition();
        worldPos.y += halfHeight;

        // Register as static collision object
        _object = physics->createCollisionObject(_shape.get(), worldPos, false);
    }

    void HeightmapCollider::update(float deltaTime)
    {
        if (!_shape || !_object)
        {
            return;
        }

        glm::vec3 worldScale = owner->transform.getWorldScale();
        float planeRes = static_cast<float>(heightmap->getWidth() - 1);
        float doubleScale = planeLen / planeRes;
        _shape->setLocalScaling(btVector3(
            doubleScale * worldScale.x,
            worldScale.y,
            doubleScale * worldScale.z
        ));

        btTransform t;
        t.setIdentity();

        glm::vec3 worldPos = owner->transform.getWorldPosition();
        worldPos.y += heightmap->getHeightScale() * 0.5f * worldScale.y;
        t.setOrigin(PhysicsSystem::toBullet(worldPos));
        t.setRotation(PhysicsSystem::toBullet(owner->transform.getWorldRotation()));
        _object->setWorldTransform(t);
    }
}