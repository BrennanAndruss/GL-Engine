#pragma once

// Forward declarations
namespace engine
{
	class AssetManager;
}

namespace engine
{
	class RenderPass
	{
	public:
		virtual	~RenderPass() = default;
		virtual void execute(AssetManager& assets) = 0;
	};
}