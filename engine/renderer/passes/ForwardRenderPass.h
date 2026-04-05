#pragma once

#include <memory>
#include "scene/Scene.h"
#include "renderer/passes/RenderPass.h"

// Forward declarations
namespace engine
{
	class AssetManager;
}

namespace engine
{
	class ForwardRenderPass : public RenderPass
	{
	public:
		ForwardRenderPass();
		void execute(const Scene& scene, const AssetManager& assets) override;
	};
}