#pragma once

#include <vector>

#include "core/Game.h"
#include "core/EventCallbacks.h"
#include "resources/AssetManager.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/Object.h"

class MyGame : public engine::Game, public engine::EventCallbacks
{
public:
	MyGame() = default;
	~MyGame() = default;

	void init(engine::AssetManager& assets, 
			  engine::Renderer& renderer, 
			  engine::Scene& scene, 
			  const engine::AppConfig& config) override;
	void update(float deltaTime) override;

private:
	engine::Object* cube = nullptr;
	engine::Object* gem = nullptr;
	engine::Object* pointLightCenter = nullptr;
	std::vector<engine::Object*> objects;
};