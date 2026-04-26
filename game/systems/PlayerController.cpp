#include "systems/PlayerController.h"

#include <GLFW/glfw3.h>
#include <cassert>
#include "core/Input.h"
#include "scene/Object.h"

void PlayerController::start()
{
	_characterController = owner->getComponent<engine::CharacterController>();
	assert(_characterController && "PlayerController requires CharacterController component");
}

void PlayerController::update(float deltaTime)
{
	// Read input
	glm::vec3 input(0.0f);
	if (engine::Input::isKeyDown(GLFW_KEY_W)) input.z += 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_S)) input.z -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_A)) input.x -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_D)) input.x += 1.0f;

	// Normalize input and transform to camera space
	if (input != glm::vec3(0.0f))
	{
		input = glm::normalize(glm::vec3(input.x, 0.0f, input.z));
		input = input.x * camera->getRight() + input.z * camera->getForward();
	}

	_characterController->move(input * moveSpeed * deltaTime);

	// Mouse look and click
	glm::vec2 mouseDelta = engine::Input::getMouseDelta();
	if (invertMouseMove) mouseDelta *= -1.0f;

	float yawDelta = 0.0f, pitchDelta = 0.0f;
	if (engine::Input::isMouseDown(GLFW_MOUSE_BUTTON_LEFT))
	{
		yawDelta = mouseDelta.x * sensitivity;
		pitchDelta = mouseDelta.y * sensitivity;
	}

	if (camera)
	{
		camera->rotate(pitchDelta, yawDelta);
		camera->setPosition(
			owner->transform.getWorldPosition() + glm::vec3(0.0f, eyeHeight, 0.0f));
	}
}