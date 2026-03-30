#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material
{
	Material() = default;
	~Material() = default;

	size_t shaderId;
	size_t difTextureId;
	size_t specTextureId;

	float shininess = 0.0f;
	glm::vec3 ambient = glm::vec3(0.0f);
	glm::vec3 diffuse = glm::vec3(0.0f);
	glm::vec3 specular = glm::vec3(0.0f);
};