#pragma once

#include <vector>
#include <memory>
#include "renderer/passes/RenderPass.h"
#include "renderer/passes/BlitPass.h"
#include "renderer/RenderContext.h"
#include "renderer/UniformBuffer.h"
#include "scene/Scene.h"

// Forward declarations
namespace engine
{
	class AssetManager;
}

namespace engine
{
	constexpr int MAX_LIGHTS = 16;

	class Renderer
	{
	public:
		Renderer(int width, int height);
		~Renderer() = default;

		void init(AssetManager& assets);

		void addRenderPass(std::unique_ptr<RenderPass> renderPass);
		void resize(int width, int height);
		void render(const Scene& scene, const AssetManager& assets);

	private:
		std::vector<std::unique_ptr<RenderPass>> _renderPasses;
		std::unique_ptr<BlitPass> _blitPass;
		
		int _width, _height;
		RenderContext _ctx;
		UniformBuffer _cameraUBO, _lightsUBO;
	};
}