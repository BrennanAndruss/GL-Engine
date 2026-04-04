#include "Renderer.h"

#include <glad/glad.h>
#include <cassert>

namespace engine
{
	Renderer::Renderer(int width, int height) : _width(width), _height(height) {}

	void Renderer::addRenderPass(std::unique_ptr<RenderPass> pass)
	{
		assert(pass != nullptr && "Null render pass.");
		_renderPasses.push_back(std::move(pass));
	}

	void Renderer::resize(int width, int height)
	{
		_width = width;
		_height = height;
	}

	void Renderer::render(AssetManager& assets)
	{
		// Clear the previous frame
		glViewport(0, 0, _width, _height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (const auto& pass : _renderPasses)
		{
			pass->execute(assets);
		}
	}
}
