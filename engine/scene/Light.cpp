#include "scene/Light.h"

#include <cassert>

namespace engine
{
	Light::Light(glm::vec3 color, float intensity) : _intensity(intensity) 
	{
		setColor(color);
	}

	DirectionalLight::DirectionalLight(glm::vec3 color, float intensity, glm::vec3 direction) :
		Light(color, intensity)
	{
		setDirection(direction);
	}

	void DirectionalLight::setDirection(glm::vec3 direction)
	{
		assert(glm::length(direction) > 0.0f && "Direction cannot be zero vector.");
		_walkDirection = glm::normalize(direction);
	}

	LightData DirectionalLight::getLightData() const
	{
		return {
			glm::vec4(_color, _intensity),
			glm::vec4(0.0f),
			glm::vec4(glm::normalize(_walkDirection), static_cast<float>(LightType::Directional))
		};
	}

	PointLight::PointLight(glm::vec3 color, float intensity, glm::vec3 position, float range) :
		Light(color, intensity), _position(position), _range(range) {}

	LightData PointLight::getLightData() const
	{
		return {
			glm::vec4(_color, _intensity),
			glm::vec4(_position, _range),
			glm::vec4(0.0f, 0.0f, 0.0f, static_cast<float>(LightType::Point))
		};
	}
}