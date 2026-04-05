#pragma once

#include <vector>
#include <memory>
#include "renderer/passes/RenderPass.h"
#include "renderer/UniformBuffer.h"
#include "scene/Scene.h"
#include "resources/AssetManager.h"

namespace engine
{
	constexpr int MAX_LIGHTS = 16;

	class Renderer
	{
	public:
		Renderer(int width, int height);
		~Renderer() = default;

		void addRenderPass(std::unique_ptr<RenderPass> renderPass);
		void resize(int width, int height);
		void render(const Scene& scene, const AssetManager& assets);

	private:
		std::vector<std::unique_ptr<RenderPass>> _renderPasses;
		
		int _width, _height;
		UniformBuffer _cameraUBO, _lightsUBO;
	};
}