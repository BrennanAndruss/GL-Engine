#include "MyGame.h"

#include <memory>
#include "resources/AssetManager.h"
#include "renderer/resources/Shader.h"
#include "renderer/passes/ForwardRenderPass.h"


void MyGame::init(engine::AssetManager& assets, engine::Renderer& renderer)
{
	// Load assets
	auto test = assets.loadShader("test", "shaders/test.vert", "shaders/test.frag");
	
	// Configure render pipeline
	renderer.addRenderPass(std::make_unique<engine::ForwardRenderPass>());
}

void MyGame::update(float deltaTime)
{
	// Game logic
}