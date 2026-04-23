#pragma once

#include <vector>
#include <memory>
#include "resources/Heightmap.h"
#include "scene/components/Collider.h"

// Forward declarations
class btCollisionShape;
class btHeightfieldTerrainShape;

namespace engine
{
    class HeightmapCollider : public Collider
    {
    public:
        // Explicit constructors to handle forward declarations
        HeightmapCollider();
        ~HeightmapCollider() override;

        Heightmap* heightmap = nullptr;
        float planeLen = 100.0f;

        void start() override;
        btCollisionShape* getShape() const override;

    private:
        std::unique_ptr<btHeightfieldTerrainShape> _shape;
    };
}