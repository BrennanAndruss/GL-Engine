#pragma once

#include "renderer/passes/RenderPass.h"

// Forward declarations
namespace engine
{
	class AssetManager;
	class Shader;
}

namespace engine
{
	class SkyboxRenderPass : public RenderPass
	{
	public:
		SkyboxRenderPass(Handle<Shader> shader);
		~SkyboxRenderPass() override;

		void execute(const Scene& scene, const AssetManager& assets) override;

	private:
		unsigned int _vao = 0;
		unsigned int _vbo = 0;
	};
}