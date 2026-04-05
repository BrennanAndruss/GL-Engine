#pragma once

// Forward declarations
namespace engine
{
	class Scene;
	class AssetManager;
}

namespace engine
{
	class RenderPass
	{
	public:
		virtual	~RenderPass() = default;
		virtual void execute(const Scene& scene, const AssetManager& assets) = 0;
	};
}