#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace engine
{
	struct Material
	{
		size_t shaderId = 0;
		size_t difTextureId = 0;
		size_t specTextureId = 0;

		float shininess = 0.0f;
		glm::vec3 ambient = glm::vec3(0.0f);
		glm::vec3 diffuse = glm::vec3(0.0f);
		glm::vec3 specular = glm::vec3(0.0f);
	};
}