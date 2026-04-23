#pragma once

#include <string>
#include <vector>
#include <cstddef>

namespace engine
{
	class Heightmap
	{
	public:
		Heightmap(int width, int length, int channels, 
			const unsigned char* heights, float heightScale = 1.0f);
		~Heightmap();

		bool loadFromFile(const std::string& path);

		float sample(int x, int y) const;
		float sample(float u, float v) const;

		int getWidth() const { return _width; }
		int getLength() const { return _length; }
		int getChannels() const { return _channels; }
		float getHeightScale() const { return _heightScale; }

		const std::vector<unsigned char>& getPixels() const { return _pixels; }
		const std::vector<float>& getHeightData() const { return _heightData; }

	private:
		int _width = 0;
		int _length = 0;
		int _channels = 0;
		float _heightScale = 1.0f;
		std::vector<unsigned char> _pixels;
		std::vector<float> _heightData;

		void generateHeightData();
	};
}