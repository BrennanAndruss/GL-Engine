#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace engine
{
	glm::mat4 Transform::getMatrix() const
	{
		// Compute composite transform
		glm::mat4 model(1.0f);

		model = glm::translate(model, translation);
		model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scale);

		return model;
	}
}