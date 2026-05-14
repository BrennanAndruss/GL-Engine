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
	float moveSpeed = 28.0f;
	float rotationSpeed = 1.0f;
	float eyeHeight = 0.4f;
	float cameraDistance = 4.5f;
	float sensitivity = 0.1f;
	float jumpForce = 32.0f;
	bool invertMouseMove = false;
	bool enabled = true;
	bool hasJumped = false;

	engine::Transform* cameraTransform = nullptr;
	
	void start() override;
	void update(float deltaTime) override;
	engine::Animator* animator = nullptr;
	Handle<engine::AnimationClip> idleClip;
	Handle<engine::AnimationClip> sprintClip;
	Handle<engine::AnimationClip> jumpClip;

private:
	engine::CharacterController* _characterController = nullptr;

	float _yaw = 0.0f;
	float _pitch = 0.0f;
};