#pragma once

#include <glad/glad.h>
#include <cstddef>

namespace engine
{
	class UniformBuffer
	{
	public:
		UniformBuffer(std::size_t size, GLuint bindingPoint);
		~UniformBuffer();

		void update(const void* data, std::size_t size, std::size_t offset = 0);

	private:
		GLuint _uboId;
	};
}