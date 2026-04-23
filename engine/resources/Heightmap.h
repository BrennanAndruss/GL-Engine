#pragma once

#include <string>
#include <vector>

namespace engine
{
	class Heightmap
	{
	public:
		bool loadFromFile(const std::string& path);

		float sample(int x, int y) const;
		float sample(float u, float v) const;

		int getWidth() const { return _width; }
		int getHeight() const { return _height; }
		int getChannels() const { return _channels; }

	private:
		int _width = 0;
		int _height = 0;
		int _channels = 0;
		std::vector<unsigned char> _pixels;
	};
}