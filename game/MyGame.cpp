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
#include "scene/components/Components.h"
#include "systems/PlayerController.h"
#include "systems/Collectable.h"

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
	size_t grayMatId = assets.loadMaterial("grayMat");
	auto* grayMat = assets.getMaterial(grayMatId);
	grayMat->shaderId = shaderId;
	grayMat->ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	grayMat->diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	grayMat->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	grayMat->shininess = 32.0f;

	size_t redMatId = assets.loadMaterial("redMat");
	auto* redMat = assets.getMaterial(redMatId);
	redMat->shaderId = shaderId;
	redMat->ambient = glm::vec3(0.2f, 0.0f, 0.0f);
	redMat->diffuse = glm::vec3(0.8f, 0.0f, 0.0f);
	redMat->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	redMat->shininess = 32.0f;

	size_t greenMatId = assets.loadMaterial("greenMat");
	auto* greenMat = assets.getMaterial(greenMatId);
	greenMat->shaderId = shaderId;
	greenMat->ambient = glm::vec3(0.0f, 0.2f, 0.0f);
	greenMat->diffuse = glm::vec3(0.0f, 0.8f, 0.0f);
	greenMat->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	greenMat->shininess = 32.0f;

	// Initialize scene
	{
		float aspect = static_cast<float>(config.width) / static_cast<float>(config.height);
		auto& camera = scene.createCamera(glm::vec3(0.0f, 1.0f, 5.0f), 45.0f, aspect);
	}

	{
		auto& dirLight = scene.createLight<engine::DirectionalLight>();
		dirLight.setDirection(glm::vec3(0.8f, -1.0f, 0.6f));
		dirLight.setColor(glm::vec3(1.0f));
		dirLight.setIntensity(1.0f);
	}

	{
		auto& pointLight1 = scene.createLight<engine::PointLight>();
		pointLight1.setPosition(glm::vec3(2.0f, 2.0f, 2.0f));
		pointLight1.setRange(10.0f);
		pointLight1.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
		pointLight1.setIntensity(1.0f);
	}

	{
		auto& pointLight2 = scene.createLight<engine::PointLight>();
		pointLight2.setPosition(glm::vec3(-2.0f, 2.0f, -2.0f));
		pointLight2.setRange(10.0f);
		pointLight2.setColor(glm::vec3(0.0f, 0.0f, 1.0f));
		pointLight2.setIntensity(1.0f);
	}

	{
		auto& pointLight3 = scene.createLight<engine::PointLight>();
		pointLight3.setPosition(glm::vec3(0.0f, 0.5f, 0.0f));
		pointLight3.setRange(10.0f);
		pointLight3.setColor(glm::vec3(0.0f, 1.0f, 0.0f));
		pointLight3.setIntensity(1.0f);
	}
	
	{
		cube = &scene.createObject("Cube");
		cube->transform.translation = glm::vec3(0.0f, 2.0f, 0.0f);

		auto& meshRenderer = cube->addComponent<engine::MeshRenderer>();
		meshRenderer.meshId = cubeMeshId;
		meshRenderer.materialId = grayMatId;

		//cube->addComponent<engine::BoxCollider>();
		//cube->addComponent<engine::RigidBody>();
	}

	{
		auto& floor = scene.createObject("Floor");
		floor.transform.translation = glm::vec3(0.0f, -2.0f, 0.0f);
		floor.transform.scale = glm::vec3(20.0f, 2.0f, 20.0f);

		auto& collider = floor.addComponent<engine::BoxCollider>();
		collider.size = floor.transform.scale;

		auto& meshRenderer = floor.addComponent<engine::MeshRenderer>();
		meshRenderer.meshId = cubeMeshId;
		meshRenderer.materialId = grayMatId;
	}

	// Initialize player
	{
		auto& player = scene.createObject("Player");
		player.transform.translation = glm::vec3(0.0f, 5.0f, 5.0f);

		auto& charController = player.addComponent<engine::CharacterController>();
		charController.height = 1.0f;

		auto& playerController = player.addComponent<PlayerController>();
		playerController.camera = scene.getCamera();
		playerController.moveSpeed = 10.0f;
		playerController.eyeHeight = 0.5f;
	}

	// Initialize collectables
	for (int i = 0; i < 20; i++)
	{
		auto& obj = scene.createObject("Collectable" + i);
		obj.transform.translation = glm::vec3(i * 2.0f - 20.0f, 1.0f, -5.0f);
		obj.transform.scale = glm::vec3(0.25f);

		auto& mr = obj.addComponent<engine::MeshRenderer>();
		mr.meshId = cubeMeshId;
		mr.materialId = redMatId;;

		auto& collider = obj.addComponent<engine::BoxCollider>();
		collider.size = obj.transform.scale;
		collider.isTrigger = true;

		auto& collectable = obj.addComponent<Collectable>();
		collectable.collectedMatId = greenMatId;
	}

	// Configure render pipeline
	renderer.addRenderPass(std::make_unique<engine::ForwardRenderPass>());

	std::cout << "Game initialized!\n";
}

void MyGame::update(float deltaTime)
{
	cube->transform.rotation += glm::vec3(1.0f, 0.0f, 1.0f) * (deltaTime * 5);
}