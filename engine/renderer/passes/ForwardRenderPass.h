#pragma once

#include <memory>
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
		void execute(AssetManager& assets) override;
	};
}