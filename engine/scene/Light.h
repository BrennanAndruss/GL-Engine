#pragma once

#include <glm/glm.hpp>

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

	class Light
	{
	public:
		Light() = default;
		Light(glm::vec3 color, float intensity);
		virtual ~Light() = default;

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
		DirectionalLight() = default;
		DirectionalLight(glm::vec3 color, float intensity, glm::vec3 direction);

		void setDirection(glm::vec3 direction);
		LightData getLightData() const override;

	private:
		glm::vec3 _direction;
	};

	class PointLight : public Light
	{
	public:
		PointLight() = default;
		PointLight(glm::vec3 color, float intensity, glm::vec3 position, float range);

		void setPosition(glm::vec3 position) { _position = position; }
		void setRange(float range) { _range = range; }
		LightData getLightData() const override;

	private:
		glm::vec3 _position;
		float _range;
	};
}