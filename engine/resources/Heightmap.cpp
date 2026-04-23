#include "resources/Heightmap.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace engine
{
	Heightmap::Heightmap(int width, int length, int channels, const unsigned char* heights, float heightScale)
		: _width(width), _length(length), _channels(channels), _heightScale(heightScale)
	{
		std::size_t pixelCount = static_cast<std::size_t>(_width) * static_cast<std::size_t>(_length) * static_cast<std::size_t>(_channels);
		_pixels.assign(heights, heights + pixelCount);

		// Generate height data from pixel data
		if (_pixels.empty()) return;

		_heightData.clear();
		_heightData.reserve(_width * _length);

		for (int y = 0; y < _length; y++)
		{
			for (int x = 0; x < _width; x++)
			{
				// Use first channel as height
				std::size_t index = (y * _width + x) * _channels;
				float normalizedHeight = static_cast<float>(_pixels[index]) / 255.0f;
				_heightData.emplace_back(normalizedHeight * _heightScale);
			}
		}
	}

	Heightmap::~Heightmap() = default;

	float Heightmap::sample(int x, int y) const
	{
		if (_pixels.empty() || _width <= 0 || _length <= 0 || _channels <= 0)
		{
			return 0.0f;
		}

		x = std::clamp(x, 0, _width - 1);
		y = std::clamp(y, 0, _length - 1);

		// Use first channel as height
		std::size_t index = (static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)) * _channels;

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
		int y = static_cast<int>(std::round(v * (_length - 1)));

		return sample(x, y);
	}
}