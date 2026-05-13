#include "resources/Heightmap.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace engine
{
	Heightmap::Heightmap(int width, int length, int channels, const unsigned char* heights, float heightScale)
		: _width(width), _length(length), _channels(channels), _heightScale(heightScale)
	{
		std::size_t pixelCount = static_cast<std::size_t>(_width) * static_cast<std::size_t>(_length) * static_cast<std::size_t>(_channels);
		_pixels.assign(heights, heights + pixelCount);

		// Generate normalized height data from pixel data and apply a small blur to smooth the terrain
		if (_pixels.empty()) return;

		_heightData.clear();
		_heightData.reserve(_width * _length);

		// Build normalized buffer (0..1) from first channel
		std::vector<float> norm;
		norm.resize(static_cast<std::size_t>(_width) * static_cast<std::size_t>(_length));

		for (int y = 0; y < _length; ++y)
		{
			for (int x = 0; x < _width; ++x)
			{
				std::size_t pIndex = (static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)) * _channels;
				norm[static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)] =
					static_cast<float>(_pixels[pIndex]) / 255.0f;
			}
		}

		// Separable 3x3 Gaussian-like blur (weights 1,2,1) performed horizontally then vertically
		std::vector<float> temp(norm.size());

		for (int y = 0; y < _length; ++y)
		{
			for (int x = 0; x < _width; ++x)
			{
				int xm1 = std::max(0, x - 1);
				int xp1 = std::min(_width - 1, x + 1);
				float left = norm[static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(xm1)];
				float center = norm[static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)];
				float right = norm[static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(xp1)];
				temp[static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)] = (left + 2.0f * center + right) * 0.25f;
			}
		}

		for (int y = 0; y < _length; ++y)
		{
			for (int x = 0; x < _width; ++x)
			{
				int ym1 = std::max(0, y - 1);
				int yp1 = std::min(_length - 1, y + 1);
				float top = temp[static_cast<std::size_t>(ym1) * _width + static_cast<std::size_t>(x)];
				float center = temp[static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)];
				float bottom = temp[static_cast<std::size_t>(yp1) * _width + static_cast<std::size_t>(x)];
				norm[static_cast<std::size_t>(y) * _width + static_cast<std::size_t>(x)] = (top + 2.0f * center + bottom) * 0.25f;
			}
		}

		// Store normalized (0..1) heights in _heightData. Mesh code multiplies by heightScale when needed.
		_heightData = std::move(norm);
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

		// Read from precomputed (and blurred) normalized height data
		std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(_width) + static_cast<std::size_t>(x);
		if (index >= _heightData.size()) return 0.0f;
		return _heightData[index];
	}

	float Heightmap::sample(float u, float v) const
	{
		if (_pixels.empty())
		{
			return 0.0f;
		}

		u = std::clamp(u, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		// Bilinear filtering: map normalized UV to texel space and interpolate
		float fx = u * static_cast<float>(_width - 1);
		float fy = v * static_cast<float>(_length - 1);

		int x0 = static_cast<int>(std::floor(fx));
		int y0 = static_cast<int>(std::floor(fy));
		int x1 = std::min(x0 + 1, _width - 1);
		int y1 = std::min(y0 + 1, _length - 1);

		float sx = fx - static_cast<float>(x0);
		float sy = fy - static_cast<float>(y0);

		// sample(int,int) returns normalized [0..1] value from precomputed _heightData
		float h00 = sample(x0, y0);
		float h10 = sample(x1, y0);
		float h01 = sample(x0, y1);
		float h11 = sample(x1, y1);

		float hx0 = h00 + (h10 - h00) * sx;
		float hx1 = h01 + (h11 - h01) * sx;
		return hx0 + (hx1 - hx0) * sy;
	}
}