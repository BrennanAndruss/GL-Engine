#include "systems/PlayerController.h"

#include <GLFW/glfw3.h>
#include <cassert>
#include "core/Input.h"
#include "scene/Object.h"

void PlayerController::start()
{
	_characterController = owner->getComponent<engine::CharacterController>();
	assert(_characterController && "PlayerController requires CharacterController component");

	assert(cameraTransform && "PlayerController requires Camera component transform");
	cameraTransform->setPosition(0.0f, eyeHeight, 0.0f);
}

void PlayerController::update(float deltaTime)
{
	// Mouse look and click
	glm::vec2 mouseDelta = engine::Input::getMouseDelta();
	if (invertMouseMove) mouseDelta.y *= -1.0f;

	if (engine::Input::isMouseDown(GLFW_MOUSE_BUTTON_LEFT))
	{
		_yaw -= mouseDelta.x * sensitivity;
		_pitch += mouseDelta.y * sensitivity;
		_pitch = glm::clamp(_pitch, -89.0f, 89.0f);
	}

	// Rotate the player body for yaw
	glm::quat yawQuat = glm::angleAxis(glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	owner->transform.setRotation(yawQuat);

	// Rotate the camera for pitch
	glm::quat pitchQuat = glm::angleAxis(glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	cameraTransform->setRotation(pitchQuat);

	// WASD movement
	glm::vec3 input(0.0f);
	if (engine::Input::isKeyDown(GLFW_KEY_W)) input.z += 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_S)) input.z -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_A)) input.x -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_D)) input.x += 1.0f;

	// Normalize input and transform to camera space
	if (input != glm::vec3(0.0f))
	{
		input = glm::normalize(glm::vec3(input.x, 0.0f, input.z));
		glm::vec3 walkDir = input.x * owner->transform.getRight() + 
			input.z * owner->transform.getForward();
		
		walkDir.y = 0.0f;
		if (walkDir != glm::vec3(0.0f))
		{
			walkDir = glm::normalize(walkDir);
		}

		_characterController->move(walkDir * moveSpeed * deltaTime);
	}
}