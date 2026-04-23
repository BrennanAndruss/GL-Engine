#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace engine
{
	void Transform::setPosition(glm::vec3 position)
	{
		_position = position;
		markDirty();
	}

	void Transform::translate(glm::vec3 position)
	{
		_position += position;
		markDirty();
	}

	void Transform::setScale(glm::vec3 scale)
	{
		_scale = scale;
		markDirty();
	}

	void Transform::setEulerAngles(glm::vec3 eulerAngles)
	{
		_rotation = glm::quat(glm::radians(eulerAngles));
		markDirty();
	}

	glm::vec3 Transform::getEulerAngles() const
	{
		return glm::degrees(glm::eulerAngles(_rotation));
	}

	void Transform::rotate(glm::vec3 eulerAngles)
	{
		glm::quat delta = glm::quat(glm::radians(eulerAngles));
		_rotation = glm::normalize(_rotation * delta);
		markDirty();
	}

	void Transform::rotate(float angleDegrees, glm::vec3 axis)
	{
		glm::quat delta = glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));
		_rotation = glm::normalize(_rotation * delta);
		markDirty();
	}

	void Transform::rotateAround(glm::vec3 point, glm::vec3 axis, float angleDegrees)
	{
		glm::quat delta = glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));

		// Translate to rotation point, apply rotation, then translate back the same distance
		glm::vec3 offset = _position - point;
		offset = delta * offset;
		_position = point + offset;
		_rotation = glm::normalize(_rotation * delta);
		markDirty();
	}

	void Transform::setRotation(glm::quat q)
	{
		_rotation = q;
		markDirty();
	}

	glm::mat4 Transform::getLocalMatrix() const
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, _position);
		model *= glm::mat4_cast(_rotation);
		model = glm::scale(model, _scale);
		return model;
	}

	const glm::mat4& Transform::getWorldMatrix() const
	{
		if (_dirty)
		{
			updateWorldMatrix();
		}
		return _worldMatrix;
	}

	void Transform::updateWorldMatrix() const
	{
		if (_parent)
		{
			_worldMatrix = _parent->getWorldMatrix() * getLocalMatrix();
		}
		else
		{
			_worldMatrix = getLocalMatrix();
		}
		_dirty = false;
	}

	glm::vec3 Transform::getWorldPosition() const
	{
		return glm::vec3(getWorldMatrix()[3]);
	}

	glm::quat Transform::getWorldRotation() const
	{
		if (_parent)
		{
			return _parent->getWorldRotation() * _rotation;
		}
		return _rotation;
	}

	glm::vec3 Transform::getWorldScale() const
	{
		const glm::mat4& worldMatrix = getWorldMatrix();
		return glm::vec3(
			glm::length(glm::vec3(worldMatrix[0])),
			glm::length(glm::vec3(worldMatrix[1])),
			glm::length(glm::vec3(worldMatrix[2]))
		);
	}

	void Transform::setParent(Transform* parent, bool worldPositionStays)
	{
		if (_parent)
		{
			_parent->removeChild(this);
		}

		if (worldPositionStays && parent)
		{
			// Convert current world transform to local space of new parent
			glm::mat4 worldMatrix = getWorldMatrix();
			glm::mat4 parentInv = glm::inverse(parent->getWorldMatrix());
			glm::mat4 localMatrix = parentInv * worldMatrix;

			// Decompose new local matrix to position, rotation, scale
			_position = glm::vec3(localMatrix[3]);
			_scale = glm::vec3(
				glm::length(glm::vec3(localMatrix[0])),
				glm::length(glm::vec3(localMatrix[1])),
				glm::length(glm::vec3(localMatrix[2]))
			);

			glm::mat3 rotationMatrix(
				glm::vec3(localMatrix[0]) / _scale.x,
				glm::vec3(localMatrix[1]) / _scale.y,
				glm::vec3(localMatrix[2]) / _scale.z
			);
			_rotation = glm::quat_cast(rotationMatrix);
		}

		_parent = parent;
		if (_parent)
		{
			_parent->addChild(this);
		}

		markDirty();
	}

	void Transform::addChild(Transform* child)
	{
		_children.push_back(child);
	}

	void Transform::removeChild(Transform* child)
	{
		_children.erase(
			std::remove(_children.begin(), _children.end(), child),
			_children.end()
		);
	}

	void Transform::markDirty()
	{
		if (_dirty) return;

		_dirty = true;
		for (auto& child : _children)
		{
			child->markDirty();
		}
	}
}