#include "scene/Camera.h"

namespace engine
{
	Camera::Camera(float fov, float aspect, float near, float far) :
		_fov(fov), _aspect(aspect), _near(near), _far(far), _cameraData() {}

	void Camera::start()
	{
		updateViewMatrix();
		updateProjectionMatrix();
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

	void Camera::updateViewMatrix()
	{
		_cameraData.view = glm::inverse(owner->transform.getWorldMatrix());
		_cameraData.position = glm::vec4(owner->transform.getWorldPosition(), 1.0f);
	}

	void Camera::updateProjectionMatrix()
	{
		_cameraData.projection = glm::perspective(glm::radians(_fov), _aspect, _near, _far);
	}
}