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
	auto* grayMat = assets.getMaterial(matId);
	grayMat->shaderId = shaderId;
	grayMat->ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	grayMat->diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	grayMat->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	grayMat->shininess = 32.0f;

	// Initialize scene
	float aspect = static_cast<float>(config.width) / static_cast<float>(config.height);
	auto& camera = scene.createCamera(glm::vec3(-5.0f, 0.0f, 0.0f), 45.0f, aspect);

	auto& dirLight = scene.createLight<engine::DirectionalLight>();
	dirLight.setDirection(glm::vec3(0.8f, -1.0f, 0.6f));
	dirLight.setColor(glm::vec3(1.0f));
	dirLight.setIntensity(1.0f);

	auto& pointLight1 = scene.createLight<engine::PointLight>();
	pointLight1.setPosition(glm::vec3(-1.0f, -1.0f, 2.0f));
	pointLight1.setRange(10.0f);
	pointLight1.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
	pointLight1.setIntensity(1.0f);

	auto& pointLight2 = scene.createLight<engine::PointLight>();
	pointLight2.setPosition(glm::vec3(-1.0f, -1.0f, -2.0f));
	pointLight2.setRange(10.0f);
	pointLight2.setColor(glm::vec3(0.0f, 0.0f, 1.0f));
	pointLight2.setIntensity(1.0f);

	auto& pointLight3 = scene.createLight<engine::PointLight>();
	pointLight3.setPosition(glm::vec3(-1.0f, 2.0f, 0.0f));
	pointLight3.setRange(10.0f);
	pointLight3.setColor(glm::vec3(0.0f, 1.0f, 0.0f));
	pointLight3.setIntensity(1.0f);
	
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