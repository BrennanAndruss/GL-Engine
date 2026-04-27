#pragma once

#include <glm/glm.hpp>
#include "scene/Transform.h"
#include "scene/components/Component.h"
#include "scene/components/CharacterController.h"

class PlayerController : public engine::Component
{
public:
	float moveSpeed = 4.0f;
	float rotationSpeed = 1.0f;
	float eyeHeight = 1.0f;
	float sensitivity = 0.1f;
	bool invertMouseMove = true;

	engine::Transform* cameraTransform = nullptr;
	
	void start() override;
	void update(float deltaTime) override;

private:
	engine::CharacterController* _characterController = nullptr;

	float _yaw, _pitch;
};