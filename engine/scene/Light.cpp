#include "scene/Light.h"

namespace engine
{
	DirectionalLight::DirectionalLight(glm::vec3 direction) : direction(direction) {}

	LightData DirectionalLight::getLightData() const
	{
		return {
			glm::vec4(direction, static_cast<float>(LightType::Directional))
		};
	}
}