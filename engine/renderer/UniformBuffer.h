#pragma once

#include <glad/glad.h>

namespace engine
{
	class UniformBuffer
	{
	public:
		UniformBuffer(size_t size, GLuint bindingPoint);
		~UniformBuffer();

		void update(const void* data, size_t size, size_t offset = 0);

	private:
		GLuint _uboId;
	};
}