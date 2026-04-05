#include "scene/Camera.h"

namespace engine
{
	Camera::Camera(glm::vec3 position, float fov, float aspect, float near, float far) :
		_position(position),
		_pitch(0.0f),
		_yaw(0.0f),
		_fov(fov),
		_aspect(aspect),
		_near(near),
		_far(far)
	{
		updateCameraVectors();
		updateViewMatrix();
		updateProjectionMatrix();
	}

	void Camera::setPosition(glm::vec3 position)
	{
		_position = position;
		updateViewMatrix();
	}

	void Camera::setRotation(float pitch, float yaw)
	{
		_pitch = pitch;
		_yaw = yaw;

		// Constrain pitch to avoid gimbal lock
		if (_pitch > 89.0f)
			_pitch = 89.0f;
		if (_pitch < -89.0f)
			_pitch = -89.0f;

		updateCameraVectors();
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

	void Camera::updateCameraVectors()
	{
		// Convert from degrees to radians
		float p = glm::radians(_pitch);
		float y = glm::radians(_yaw);

		// Calculate new orthonormal basis
		_forward.x = cos(p) * cos(y);
		_forward.y = sin(p);
		_forward.z = cos(p) * sin(y);
		
		_forward = glm::normalize(_forward);
		_right = glm::normalize(glm::cross(_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		_up = glm::cross(_right, _forward);
	}

	void Camera::updateViewMatrix()
	{
		_cameraData.view = glm::lookAt(_position, _position + _forward, _up);
		_cameraData.position = glm::vec4(_position, 0.0f);
	}

	void Camera::updateProjectionMatrix()
	{
		_cameraData.projection = glm::perspective(glm::radians(_fov), _aspect, _near, _far);
	}
}