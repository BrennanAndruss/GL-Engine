#pragma once

#include <glm/glm.hpp>
#include "scene/Transform.h"
#include "scene/components/Component.h"

class FreeCameraController : public engine::Component
{
public:
	float moveSpeed = 36.0f;
	float sensitivity = 1.0f;
	bool invertMouseMove = false;
	bool enabled = false;

	void start() override;
	void update(float deltaTime) override;
	void syncFromTransform(const engine::Transform& transform);

private:
	float _yaw = 0.0f;
	float _pitch = 0.0f;
	bool _wasEnabled = false;
};