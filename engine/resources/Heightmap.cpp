#include "resources/Heightmap.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <stb_image.h>

namespace engine
{
	bool Heightmap::loadFromFile(const std::string& path)
	{
		int width = 0;
		int height = 0;
		int channels = 0;

		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (!data)
		{
			return false;
		}

		_width = width;
		_height = height;
		_channels = channels;

		std::size_t pixelCount = static_cast<std::size_t>(_width) * static_cast<std::size_t>(_height) * static_cast<std::size_t>(_channels);
		_pixels.assign(data, data + pixelCount);

		stbi_image_free(data);
		return true;
	}

	float Heightmap::sample(int x, int y) const
	{
		if (_pixels.empty() || _width <= 0 || _height <= 0 || _channels <= 0)
		{
			return 0.0f;
		}

		x = std::clamp(x, 0, _width - 1);
		y = std::clamp(y, 0, _height - 1);

		std::size_t index = (static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)) * _channels;

		// Use first channel as height
		unsigned char value = _pixels[index];
		return static_cast<float>(value) / 255.0f;
	}

	float Heightmap::sample(float u, float v) const
	{
		if (_pixels.empty())
		{
			return 0.0f;
		}

		u = std::clamp(u, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		int x = static_cast<int>(std::round(u * (_width - 1)));
		int y = static_cast<int>(std::round(v * (_height - 1)));

		return sample(x, y);
	}
}