#pragma once

// Forward declarations
namespace engine
{
	class Scene;
	class AssetManager;
	class RenderContext;
	class Shader;
}

namespace engine
{
	class RenderPass
	{
	public:
		virtual	~RenderPass() = default;
		virtual void resize(int width, int height) {}
		virtual void execute(const Scene& scene, 
			const AssetManager& assets, RenderContext& ctx) = 0;
	};
}