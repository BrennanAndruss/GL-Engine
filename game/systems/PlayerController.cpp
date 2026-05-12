#include "systems/PlayerController.h"

#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <glm/gtc/quaternion.hpp>
#include "core/Input.h"
#include "scene/Object.h"


void PlayerController::start()
{
	_characterController = owner->getComponent<engine::CharacterController>();
	assert(_characterController && "PlayerController requires CharacterController component");

	assert(cameraTransform && "PlayerController requires Camera component transform");
	const glm::vec3 focus = owner->transform.getPosition() + glm::vec3(0.0f, eyeHeight, 0.0f);
	const glm::quat yawQuat = glm::angleAxis(glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::quat pitchQuat = glm::angleAxis(glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::vec3 offset = yawQuat * pitchQuat * glm::vec3(0.0f, eyeHeight, -cameraDistance);
	cameraTransform->setPosition(focus + offset);
	cameraTransform->lookAt(focus);
}

void PlayerController::update(float deltaTime)
{
	if (!enabled)
	{
		return;
	}

	

	// Mouse look and click
	glm::vec2 mouseDelta = engine::Input::getMouseDelta();
	if (invertMouseMove) mouseDelta.y *= -1.0f;

	//if (engine::Input::isMouseDown(GLFW_MOUSE_BUTTON_LEFT))
	{
		_yaw -= mouseDelta.x * sensitivity;
		_pitch += mouseDelta.y * sensitivity;
		_pitch = glm::clamp(_pitch, -89.0f, 89.0f);
	}

	glm::quat yawQuat = glm::angleAxis(glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f));

	// Orbit the camera around the player
	glm::vec3 focus = owner->transform.getPosition() + glm::vec3(0.0f, eyeHeight, 0.0f);
	glm::quat pitchQuat = glm::angleAxis(glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 cameraOffset = yawQuat * pitchQuat * glm::vec3(0.0f, eyeHeight, -cameraDistance);
	cameraTransform->setPosition(focus + cameraOffset);
	cameraTransform->lookAt(focus);

	// WASD movement
	glm::vec3 input(0.0f);
	if (engine::Input::isKeyDown(GLFW_KEY_W)) input.z += 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_S)) input.z -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_A)) input.x -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_D)) input.x += 1.0f;

	if (_characterController->isOnGround()) {
		hasJumped = false;
	}

	if (engine::Input::isKeyDown(GLFW_KEY_SPACE) && !hasJumped && _characterController->canJump())
	{
		std::cout << "Jump!\n";
		hasJumped = true;
		_characterController->jump(glm::vec3(0.0f, jumpForce, 0.0f));
	}
	
	// Normalize input and transform to camera space
	glm::vec3 walkDir = glm::vec3(0.0f);
	if (input != glm::vec3(0.0f))
	{
		input = glm::normalize(glm::vec3(input.x, 0.0f, input.z));
		walkDir = input.x * (yawQuat * glm::vec3(-1.0f, 0.0f, 0.0f)) +
			input.z * (yawQuat * glm::vec3(0.0f, 0.0f, 1.0f));
		
		walkDir.y = 0.0f;
		if (walkDir != glm::vec3(0.0f))
		{
			walkDir = glm::normalize(walkDir);
		}

		glm::vec3 facingDir = yawQuat * glm::vec3(0.0f, 0.0f, -1.0f);
		facingDir.y = 0.0f;
		facingDir = glm::normalize(facingDir);

		owner->transform.setRotation(
		glm::quatLookAt(facingDir, glm::vec3(0.0f, 1.0f, 0.0f))
		);
	}
	
	// Apply horizontal movement through character controller
	_characterController->move(walkDir * moveSpeed * deltaTime);
}
