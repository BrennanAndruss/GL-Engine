#pragma once

#include <glm/glm.hpp>
#include "scene/Transform.h"
#include "scene/components/Component.h"
#include "scene/components/CharacterController.h"
#include "resources/Handle.h"
#include "resources/SkeletalAnimation.h"
#include "scene/components/Animator.h"

class PlayerController : public engine::Component
{
public:
	float moveSpeed = 11.5f;
	float rotationSpeed = 1.0f;
	float eyeHeight = 2.5f;
	float cameraDistance = 6.0f;
	float sensitivity = 0.1f;
	float jumpForce = 16.0f;
	bool invertMouseMove = false;
	bool enabled = true;
	bool hasJumped = false;

	engine::Transform* cameraTransform = nullptr;
	
	void start() override;
	void update(float deltaTime) override;
	engine::Animator* animator = nullptr;
	Handle<engine::AnimationClip> idleClip;
	Handle<engine::AnimationClip> sprintClip;

private:
	engine::CharacterController* _characterController = nullptr;

	float _yaw = 0.0f;
	float _pitch = 0.0f;
};