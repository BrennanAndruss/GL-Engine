#pragma once

#include "core/Game.h"
#include "core/EventCallbacks.h"
#include "resources/AssetManager.h"
#include "renderer/Renderer.h"

class MyGame : public engine::Game, public engine::EventCallbacks
{
public:
	MyGame() = default;
	~MyGame() = default;

	void init(engine::AssetManager& assets, engine::Renderer& renderer) override;
	void update(float deltaTime) override;

private:

};