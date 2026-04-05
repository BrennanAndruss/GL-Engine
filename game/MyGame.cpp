#include "MyGame.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "resources/AssetManager.h"
#include "renderer/resources/Shader.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "renderer/passes/ForwardRenderPass.h"
#include "scene/Light.h"
#include "scene/Component.h"

void MyGame::init(engine::AssetManager& assets, 
				  engine::Renderer& renderer, 
				  engine::Scene& scene,
				  const engine::AppConfig& config)
{
	// Initialize resources
	std::cout << "Loading shaders...\n";
	size_t shaderId = assets.loadShader("simple", "shaders/simple.vert", "shaders/simple.frag");

	std::cout << "Loading models...\n";
	size_t cubeMeshId = assets.loadMesh("cube", "models/cube.obj");

	std::cout << "Loading materials...\n";
	size_t matId = assets.loadMaterial("redMat");
	auto* redMat = assets.getMaterial(matId);
	redMat->shaderId = shaderId;
	redMat->ambient = glm::vec3(0.2f, 0.0f, 0.0f);
	redMat->diffuse = glm::vec3(0.8f, 0.0f, 0.0f);
	redMat->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	redMat->shininess = 32.0f;

	// Initialize scene
	float aspect = static_cast<float>(config.width) / static_cast<float>(config.height);
	auto& camera = scene.createCamera(glm::vec3(-5.0f, 0.0f, 0.0f), 45.0f, aspect);

	scene.createLight<engine::DirectionalLight>(glm::vec3(0.8f, -1.0f, 0.6f));
	
	scene.reserveObjects(1);
	cube = &scene.createObject("cube");
	auto& meshRenderer = cube->addComponent<engine::MeshRenderer>();
	meshRenderer.meshId = cubeMeshId;
	meshRenderer.materialId = matId;

	// Configure render pipeline
	renderer.addRenderPass(std::make_unique<engine::ForwardRenderPass>());
}

void MyGame::update(float deltaTime)
{
	cube->transform.rotation += glm::vec3(1.0f, 0.0f, 1.0f) * (deltaTime * 5);
}