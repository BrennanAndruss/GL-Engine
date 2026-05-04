#include "systems/FreeCameraController.h"

#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
#include "core/Input.h"
#include "scene/Object.h"

namespace
{
	void setYawPitchFromForward(const glm::vec3& forward, float& yaw, float& pitch)
	{
		const glm::vec3 dir = glm::normalize(forward);
		yaw = glm::degrees(std::atan2(-dir.x, -dir.z));
		pitch = glm::degrees(std::atan2(dir.y, glm::length(glm::vec2(dir.x, dir.z))));
	}
}

void FreeCameraController::start()
{
	setYawPitchFromForward(owner->transform.getForward(), _yaw, _pitch);
	_pitch = glm::clamp(_pitch, -89.0f, 89.0f);
}

void FreeCameraController::syncFromTransform(const engine::Transform& transform)
{
	setYawPitchFromForward(transform.getForward(), _yaw, _pitch);
	_pitch = glm::clamp(_pitch, -89.0f, 89.0f);
	_wasEnabled = true;
}

void FreeCameraController::update(float deltaTime)
{
	if (!enabled)
	{
		_wasEnabled = false;
		return;
	}

	if (!_wasEnabled)
	{
		setYawPitchFromForward(owner->transform.getForward(), _yaw, _pitch);
		_pitch = glm::clamp(_pitch, -89.0f, 89.0f);
		_wasEnabled = true;
	}

	const float keyLookSpeed = sensitivity * 120.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_LEFT)) _yaw += keyLookSpeed * deltaTime;
	if (engine::Input::isKeyDown(GLFW_KEY_RIGHT)) _yaw -= keyLookSpeed * deltaTime;
	if (engine::Input::isKeyDown(GLFW_KEY_UP)) _pitch += keyLookSpeed * deltaTime;
	if (engine::Input::isKeyDown(GLFW_KEY_DOWN)) _pitch -= keyLookSpeed * deltaTime;
	_pitch = glm::clamp(_pitch, -89.0f, 89.0f);
    // debug print
    //std::cout << "Yaw: " << _yaw << ", Pitch: " << _pitch << std::endl;

	glm::quat yawQuat = glm::angleAxis(glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::quat pitchQuat = glm::angleAxis(glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	owner->transform.setRotation(yawQuat * pitchQuat);

	glm::vec3 input(0.0f);
	if (engine::Input::isKeyDown(GLFW_KEY_W)) input.z += 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_S)) input.z -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_A)) input.x -= 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_D)) input.x += 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_SPACE)) input.y += 1.0f;
	if (engine::Input::isKeyDown(GLFW_KEY_LEFT_SHIFT)) input.y -= 1.0f;

	if (input != glm::vec3(0.0f))
	{
		input = glm::normalize(input);
		glm::vec3 moveDir = input.x * (yawQuat * glm::vec3(1.0f, 0.0f, 0.0f)) +
			input.y * glm::vec3(0.0f, 1.0f, 0.0f) +
			input.z * (yawQuat * glm::vec3(0.0f, 0.0f, -1.0f));
		owner->transform.translate(moveDir * moveSpeed * deltaTime);
	}
}