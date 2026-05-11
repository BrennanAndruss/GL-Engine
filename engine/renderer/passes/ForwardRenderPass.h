#pragma once

#include <memory>
#include "scene/Scene.h"
#include "renderer/passes/RenderPass.h"

// Forward declarations
namespace engine
{
	class AssetManager;
	class Shader;
}

namespace engine
{
	class ForwardRenderPass : public RenderPass
	{
	public:
		ForwardRenderPass(Handle<Shader> shader);
		void execute(const Scene& scene, const AssetManager& assets) override;
	};
}