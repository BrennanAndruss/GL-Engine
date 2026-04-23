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
}