#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "resources/Handle.h"

namespace engine
{
	class Shader;
	class Texture;
}

namespace engine
{
	struct Material
	{
		Handle<Shader> shader;
		Handle<Texture> difTex;
		Handle<Texture> specTex;

		float shininess = 1.0f;
		glm::vec3 ambient = glm::vec3(0.0f);
		glm::vec3 diffuse = glm::vec3(0.0f);
		glm::vec3 specular = glm::vec3(0.0f);
	};
}