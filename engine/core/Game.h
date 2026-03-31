#pragma once

// Forward declarations
namespace engine
{
	class AssetManager;
	class Renderer;
}

namespace engine
{
	class Game
	{
	public:
		virtual ~Game() = default;
		virtual void init(AssetManager& assets, Renderer& renderer) = 0;
		virtual void update(float deltaTime) = 0;
	};
}