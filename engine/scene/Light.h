#pragma once

#include <glm/glm.hpp>

namespace engine
{
	enum class LightType
	{
		Directional,
	};

	// Light data struct for the Light UBO
	struct LightData
	{
		glm::vec4 direction_type;
	};

	struct Light
	{
		virtual ~Light() = default;
		virtual LightData getLightData() const = 0;
	};

	struct DirectionalLight : public Light
	{
		DirectionalLight() = default;
		DirectionalLight(glm::vec3 direction);

		LightData getLightData() const override;

		glm::vec3 direction;
	};
}