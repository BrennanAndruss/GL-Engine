#pragma once

#include "resources/Handle.h"

// Forward declarations
namespace engine
{
	class Scene;
	class AssetManager;
	class Shader;
}

namespace engine
{
	class RenderPass
	{
	public:
		RenderPass(Handle<Shader> shader) : _shader(shader) {}
		virtual	~RenderPass() = default;
		virtual void execute(const Scene& scene, const AssetManager& assets) = 0;

	protected:
		Handle<Shader> _shader;
	};
}