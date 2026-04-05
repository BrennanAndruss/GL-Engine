#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

		const CameraData& getCameraData() const { return _cameraData; }

	private:
		glm::vec3 _position;
		float _pitch, _yaw;
		float _fov, _aspect, _near, _far;
		glm::vec3 _forward, _up, _right;
		
		CameraData _cameraData;

		void updateCameraVectors();
		void updateViewMatrix();
		void updateProjectionMatrix();
	};
}