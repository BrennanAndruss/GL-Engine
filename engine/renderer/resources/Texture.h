#pragma once

#include <glad/glad.h>

namespace engine
{
	class Texture
	{
	public:
		Texture(int width, int height, GLint format, const unsigned char* textureData);
		~Texture();

		void setUnit(GLint unit) { _unit = unit; }
		GLint getUnit() const { return _unit; }

		void bind(GLint handle) const;
		void unbind() const;

	private:
		int _width, _height;
		GLint _internalFormat, _imageFormat;

		GLuint _tid;
		GLuint _unit;
	};
}