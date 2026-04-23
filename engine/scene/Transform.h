#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace engine
{
	class Transform
	{
	public:
		void setPosition(glm::vec3 position);
		glm::vec3 getPosition() const { return _position; }
		void translate(glm::vec3 translation);
		void translate(float x, float y, float z) { translate(glm::vec3(x, y, z)); }

		// Euler angles interface in degrees
		void setEulerAngles(glm::vec3 eulerAngles);
		glm::vec3 getEulerAngles() const;
		void rotate(glm::vec3 eulerAngles);
		void rotate(float x, float y, float z) { rotate(glm::vec3(x, y, z)); }
		void rotate(float angleDegrees, glm::vec3 axis);
		void rotateAround(glm::vec3 point, glm::vec3 axis, float angleDegrees);

		void setRotation(glm::quat q);
		glm::quat getRotation() const { return _rotation; }

		void setScale(glm::vec3 scale);
		glm::vec3 getScale() const { return _scale; }

		glm::vec3 forward() const { return _rotation * glm::vec3(0.0f, 0.0f, -1.0f); }
		glm::vec3 right() const { return _rotation * glm::vec3(1.0f, 0.0f, 0.0f); }
		glm::vec3 up() const { return _rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

		glm::mat4 getLocalMatrix() const;

		const glm::mat4& getWorldMatrix() const;
		glm::vec3 getWorldPosition() const;
		glm::quat getWorldRotation() const;
		glm::vec3 getWorldScale() const;

		// Transform hierarchy
		void setParent(Transform* parent, bool worldPositionStays = true);
		Transform* getParent() const { return _parent; }
		const std::vector<Transform*>& getChildren() const { return _children; }

		bool isDirty() const { return _dirty; }
		void markDirty();

	private:
		Transform* _parent = nullptr;
		std::vector<Transform*> _children;

		// Mutable world matrix cache
		mutable glm::mat4 _worldMatrix = glm::mat4(1.0f);
		mutable bool _dirty = true;

		glm::vec3 _position = glm::vec3(0.0f);
		glm::quat _rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 _scale = glm::vec3(1.0f);

		void addChild(Transform* child);
		void removeChild(Transform* child);
		void updateWorldMatrix() const;
	};
}