#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene/Transform.h"

namespace engine
{
	// Camera data struct for the Camera UBO
	struct CameraData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 position;		// xyz + padding for UBO
	};

	class Camera
	{
	public:
		Camera(glm::vec3 position, float fov, float aspect, float near = 0.1f, float far = 100.0f);
		~Camera() = default;

		void setPosition(glm::vec3 position);
		void setRotation(float pitch, float yaw);
		void setFov(float fov);
		void setAspect(float aspect);

		void translate(glm::vec3 translation);
		void rotate(float pitchDelta, float yawDelta);

		const CameraData& getCameraData() const { return _cameraData; }
		glm::vec3 getPosition() const { return _transform.getWorldPosition(); }
		glm::vec3 getForward() const { return _transform.forward(); }
		glm::vec3 getRight() const { return _transform.right(); }
		glm::vec3 getUp() const { return _transform.up(); }

	private:
		Transform _transform;

		float _pitch, _yaw;
		float _fov, _aspect, _near, _far;
		
		CameraData _cameraData;

		void updateViewMatrix();
		void updateProjectionMatrix();
	};
}