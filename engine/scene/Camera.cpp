#include "scene/Camera.h"

namespace engine
{
	Camera::Camera(glm::vec3 position, float fov, float aspect, float near, float far) :
		_pitch(0.0f),
		_yaw(0.0f),
		_fov(fov),
		_aspect(aspect),
		_near(near),
		_far(far)
	{
		_transform.setPosition(position);
		updateViewMatrix();
		updateProjectionMatrix();
	}

	void Camera::setPosition(glm::vec3 position)
	{
		_transform.setPosition(position);
		updateViewMatrix();
	}

	void Camera::setRotation(float pitch, float yaw)
	{
		// Clamp pitch to avoid gimbal lock
		_pitch = glm::clamp(pitch, -89.0f, 89.0f);
		_yaw = yaw;

		// Construct rotation quaternion from Euler angles
		glm::quat qPitch = glm::angleAxis(glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat qYaw = glm::angleAxis(glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		_transform.setRotation(qYaw * qPitch);

		updateViewMatrix();
	}

	void Camera::setFov(float fov)
	{
		_fov = fov;
		updateProjectionMatrix();
	}

	void Camera::setAspect(float aspect)
	{
		_aspect = aspect;
		updateProjectionMatrix();
	}

	void Camera::translate(glm::vec3 translation)
	{
		_transform.translate(translation);
		updateViewMatrix();
	}

	void Camera::rotate(float pitchDelta, float yawDelta)
	{
		setRotation(_pitch + pitchDelta, _yaw + yawDelta);
	}

	void Camera::updateViewMatrix()
	{
		_cameraData.view = glm::inverse(_transform.getWorldMatrix());
		_cameraData.position = glm::vec4(_transform.getWorldPosition(), 1.0f);
	}

	void Camera::updateProjectionMatrix()
	{
		_cameraData.projection = glm::perspective(glm::radians(_fov), _aspect, _near, _far);
	}
}