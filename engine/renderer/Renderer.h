#pragma once

#include <vector>
#include <memory>
#include "renderer/passes/RenderPass.h"
#include "resources/AssetManager.h"

namespace engine
{
	class Renderer
	{
	public:
		Renderer(int width, int height);
		~Renderer() = default;

		void addRenderPass(std::unique_ptr<RenderPass> renderPass);
		void resize(int width, int height);
		void render(AssetManager& assets);

	private:
		std::vector<std::unique_ptr<RenderPass>> _renderPasses;

		int _width, _height;
	};
}