#pragma once

#include "renderer/passes/RenderPass.h"

// Forward declarations
namespace engine
{
	class Scene;
	class AssetManager;
	class RenderContext;
}

namespace engine
{
	class BlitPass : public RenderPass
	{
	public:
		void execute(const Scene& scene, const AssetManager& assets,
			RenderContext& ctx) override;
	};
}