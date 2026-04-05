#include "renderer/UniformBuffer.h"

namespace engine
{
	UniformBuffer::UniformBuffer(size_t size, GLuint bindingPoint)
	{
		// Initialize UBO and allocate memory
		glGenBuffers(1, &_uboId);
		glBindBuffer(GL_UNIFORM_BUFFER, _uboId);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, _uboId);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	UniformBuffer::~UniformBuffer()
	{
		if (_uboId)
			glDeleteBuffers(1, &_uboId);
	}

	void UniformBuffer::update(const void* data, size_t size, size_t offset)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, _uboId);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}