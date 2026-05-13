#include "scene/components/HeightmapCollider.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <cassert>
#include <vector>
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

        const auto& sourceHeights = heightmap->getHeightData();

        _scaledHeightData.resize(sourceHeights.size());
        for (std::size_t i = 0; i < sourceHeights.size(); ++i)
        {
            _scaledHeightData[i] = sourceHeights[i] * heightmap->getHeightScale();
        }

        _shape = std::make_unique<btHeightfieldTerrainShape>(
            heightmap->getWidth(),
            heightmap->getLength(),
            _scaledHeightData.data(),
            1.0f,
            0.0f,
            heightmap->getHeightScale(),
            1,
            PHY_FLOAT,
            false
        );

        float planeRes = static_cast<float>(heightmap->getWidth() - 1);
        float terrainScale = planeLen / planeRes;

        _shape->setLocalScaling(btVector3(
            terrainScale,
            1.0f,
            terrainScale
        ));

        glm::vec3 worldPos = owner->transform.getWorldPosition();

        // Bullet centers the heightfield around (minHeight + maxHeight) / 2.
        // Your visual mesh is 0 -> heightScale, so shift the object up by half.
        worldPos.y += heightmap->getHeightScale() * 0.5f;

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
        float terrainScale = planeLen / planeRes;

        _shape->setLocalScaling(btVector3(
            terrainScale * worldScale.x,
            worldScale.y,
            terrainScale * worldScale.z
        ));

        glm::vec3 worldPos = owner->transform.getWorldPosition();
        worldPos.y += heightmap->getHeightScale() * 0.5f * worldScale.y;

        btTransform t;
        t.setIdentity();
        t.setOrigin(PhysicsSystem::toBullet(worldPos));
        t.setRotation(PhysicsSystem::toBullet(owner->transform.getWorldRotation()));

        _object->setWorldTransform(t);
    }
}