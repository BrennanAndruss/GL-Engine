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
	float eyeHeight = 2.5f;
	float cameraDistance = 6.0f;
	float sensitivity = 0.1f;
	float jumpForce = 8.0f;
	bool invertMouseMove = false;
	bool enabled = true;
	bool hasJumped = false;

	engine::Transform* cameraTransform = nullptr;
	
	void start() override;
	void update(float deltaTime) override;

private:
	engine::CharacterController* _characterController = nullptr;

	float _yaw = 0.0f;
	float _pitch = 0.0f;
};