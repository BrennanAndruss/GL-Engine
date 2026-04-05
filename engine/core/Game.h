#pragma once

#include "core/AppConfig.h"

// Forward declarations
namespace engine
{
	class AssetManager;
	class Renderer;
	class Scene;
}

namespace engine
{
	class Game
	{
	public:
		virtual ~Game() = default;
		virtual void init(AssetManager& assets, Renderer& renderer, Scene& scene, const AppConfig& config) = 0;
		virtual void update(float deltaTime) = 0;
	};
}