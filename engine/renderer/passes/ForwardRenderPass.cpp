#include "renderer/passes/ForwardRenderPass.h"

#include <glad/glad.h>
#include "renderer/resources/Shader.h"
#include "resources/AssetManager.h"

namespace engine
{
	// Test data
	static std::vector<float> vertices = {
		// positions		// colors
		-0.5f, -0.5f, 0.0f,	1.0f, 0.0f, 0.0f,	// bottom left
		 0.5f, -0.5f, 0.0f,	0.0f, 1.0f, 0.0f,	// bottom right
		 0.0f,  0.5f, 0.0f,	0.0f, 0.0f, 1.0f	// top
	};

	static GLuint vao, vbo;

	ForwardRenderPass::ForwardRenderPass()
	{
		// Initialize test data
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	void ForwardRenderPass::execute(AssetManager& assets)
	{
		auto* shader = assets.getShader("test");
		shader->bind();
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		shader->unbind();
	}
}