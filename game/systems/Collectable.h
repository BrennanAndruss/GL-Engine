#pragma once

#include "scene/components/Component.h"
#include "resources/Handle.h"
#include <glm/glm.hpp>

namespace engine
{
	struct Material;
}

namespace engine
{
    class Mesh;
}

class Collectable : public engine::Component
{
public:
    enum class Type
    {
        Cyan = 0,
        Magenta = 1,
        Yellow = 2,
        speedBoost = 3,
        JumpBoost = 4
    };

    Handle<engine::Material> defaultMat;
    Handle<engine::Material> collectedMat;
	Handle<engine::Mesh> originalMesh;
    bool isCollected = false;
    Type type = Type::Cyan;

    float respawnTimer = 0.0f;
    float respawnDuration = 0.0f;
    glm::vec3 originalScale = glm::vec3(1.0f);

    void start() override;
    void update(float deltaTime) override;
    void onCollected();

private:
    void tryRegisterCallback();
    bool _callbackRegistered = false;
};