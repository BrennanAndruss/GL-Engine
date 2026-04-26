#pragma once

#include <glm/glm.hpp>
#include "scene/components/Component.h"

namespace engine
{
	enum class LightType
	{
		Directional,
		Point,
	};

	// Light data struct for the Light UBO
	struct LightData
	{
		glm::vec4 color_intensity;
		glm::vec4 position_range;
		glm::vec4 direction_type;
	};

	class Light : public Component
	{
	public:
		virtual ~Light() = default;

		void start() override;

		void setColor(glm::vec3 color) { _color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f)); }
		void setIntensity(float intensity) { _intensity = intensity; }
		virtual LightData getLightData() const = 0;

	protected:
		glm::vec3 _color;
		float _intensity;
	};

	class DirectionalLight : public Light
	{
	public:
		LightData getLightData() const override;
	};

	class PointLight : public Light
	{
	public:
		void setRange(float range) { _range = range; }
		LightData getLightData() const override;

	private:
		float _range;
	};
}