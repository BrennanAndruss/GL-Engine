#include "MyGame.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "core/Input.h"
#include "resources/AssetManager.h"
#include "resources/Heightmap.h"
#include "renderer/resources/Shader.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "renderer/passes/ForwardRenderPass.h"
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
	Handle<engine::Shader> shader = assets.loadShader("simple", "shaders/simple.vert", "shaders/simple.frag");

	std::cout << "Loading textures...\n";
	Handle<engine::Heightmap> terrainHeightmap = assets.loadHeightmap("terrainHM", "textures/heightmaps/HM_Unity02.png", 25.0f);
	auto* heightmap = assets.getHeightmap(terrainHeightmap);

	std::cout << "Loading models...\n";
	Handle<engine::Mesh> cubeMesh = assets.loadMesh("cube", "models/cube.obj");

	int planeRes = heightmap->getWidth() / 2 - 1; // 256x256 vertices (half-resolution)
	float planeLen = 100.0f;
	Handle<engine::Mesh> terrainMesh = assets.createHeightmapMesh("terrain", terrainHeightmap, planeRes, planeLen);
	
	std::cout << "Loading materials...\n";

	// todo: move creation to assetmanager
	Handle<engine::Material> defaultMat = assets.loadMaterial("defaultMat");
	auto* mat = assets.getMaterial(defaultMat);
	mat->shader = shader;
	mat->ambient = glm::vec3(0.2f);
	mat->diffuse = glm::vec3(0.8f);
	mat->specular = glm::vec3(1.0f);
	mat->shininess = 32.0f;
	
	Handle<engine::Material> grassMat = assets.loadMaterial("grassMat");
	mat = assets.getMaterial(grassMat);
	mat->shader = shader;
	mat->ambient = glm::vec3(0.113, 0.152, 0.081);
	mat->diffuse = glm::vec3(0.565, 0.761, 0.404);
	mat->specular = glm::vec3(0.5, 0.5, 0.5);
	mat->shininess = 32.0f;

	Handle<engine::Material> redMat = assets.loadMaterial("redMat");
	mat = assets.getMaterial(redMat);
	mat->shader = shader;
	mat->ambient = glm::vec3(0.2f, 0.0f, 0.0f);
	mat->diffuse = glm::vec3(0.8f, 0.0f, 0.0f);
	mat->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	mat->shininess = 32.0f;

	Handle<engine::Material> greenMat = assets.loadMaterial("greenMat");
	mat = assets.getMaterial(greenMat);
	mat->shader = shader;
	mat->ambient = glm::vec3(0.0f, 0.2f, 0.0f);
	mat->diffuse = glm::vec3(0.0f, 0.8f, 0.0f);
	mat->specular = glm::vec3(1.0f, 1.0f, 1.0f);
	mat->shininess = 32.0f;

	// Initialize scene
	{
		float aspect = static_cast<float>(config.width) / static_cast<float>(config.height);
		auto& camera = scene.createCamera(glm::vec3(0.0f, 1.0f, 5.0f), 45.0f, aspect);
	}

	{
		auto& obj = scene.createObject("DirLight");
		auto& dirLight = obj.addComponent<engine::DirectionalLight>();
		obj.transform.lookAt(glm::vec3(0.8f, -1.0f, 0.6f));
		dirLight.setColor(glm::vec3(1.0f));
		dirLight.setIntensity(1.0f);
	}

	{
		cube = &scene.createObject("Cube");
		cube->transform.setPosition(glm::vec3(0.0f, 3.0f, -5.0f));

		auto& meshRenderer = cube->addComponent<engine::MeshRenderer>();
		meshRenderer.mesh = cubeMesh;
		meshRenderer.material = defaultMat;
	}

	pointLightCenter = &scene.createObject("PointLightCenter");
	pointLightCenter->transform.setPosition(glm::vec3(0.0f, 3.5f, -5.0f));

	{
		auto& obj = scene.createObject("PointLight1");
		obj.transform.setParent(&pointLightCenter->transform);
		obj.transform.setPosition(glm::vec3(2.5f, 0.0f, 0.0f));

		auto& pointLight1 = obj.addComponent<engine::PointLight>();
		pointLight1.setRange(10.0f);
		pointLight1.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
		pointLight1.setIntensity(1.0f);
	}

	{
		auto& obj = scene.createObject("PointLight2");
		obj.transform.setParent(&pointLightCenter->transform);
		obj.transform.setPosition(glm::vec3(0.0f, -2.5f, 0.0f));

		auto& pointLight2 = obj.addComponent<engine::PointLight>();
		pointLight2.setRange(10.0f);
		pointLight2.setColor(glm::vec3(0.0f, 0.0f, 1.0f));
		pointLight2.setIntensity(1.0f);
	}

	{
		auto& obj = scene.createObject("PointLight3");
		obj.transform.setParent(&pointLightCenter->transform);
		obj.transform.setPosition(glm::vec3(0.0f, 0.0f, 2.5f));

		auto& pointLight3 = obj.addComponent<engine::PointLight>();
		pointLight3.setRange(10.0f);
		pointLight3.setColor(glm::vec3(0.0f, 1.0f, 0.0f));
		pointLight3.setIntensity(1.0f);
	}

	{
		auto& terrain = scene.createObject("Floor");

		auto& collider = terrain.addComponent<engine::HeightmapCollider>();
		collider.heightmap = heightmap;
		collider.planeLen = planeLen;

		auto& meshRenderer = terrain.addComponent<engine::MeshRenderer>();
		meshRenderer.mesh = terrainMesh;
		meshRenderer.material = grassMat;
	}

	// Initialize player
	{
		auto& player = scene.createObject("Player");
		player.transform.setPosition(glm::vec3(5.0f, 10.0f, 0.0f));

		auto& charController = player.addComponent<engine::CharacterController>();
		charController.height = 1.0f;

		auto& playerController = player.addComponent<PlayerController>();
		playerController.camera = scene.getCamera();
		playerController.moveSpeed = 10.0f;
		playerController.eyeHeight = 0.5f;
	}

	// Initialize collectables
	for (int i = 0; i < 0; i++)
	{
		auto& obj = scene.createObject("Collectable" + std::to_string(i));
		obj.transform.setPosition(glm::vec3(i * 2.0f - 20.0f, 1.0f, -5.0f));
		obj.transform.setScale(glm::vec3(0.25f));

		auto& mr = obj.addComponent<engine::MeshRenderer>();
		mr.mesh = cubeMesh;
		mr.material = redMat;

		auto& collider = obj.addComponent<engine::BoxCollider>();
		collider.size = obj.transform.getScale();
		collider.isTrigger = true;

		auto& collectable = obj.addComponent<Collectable>();
		collectable.defaultMat = redMat;
		collectable.collectedMat = greenMat;
	}

	// Configure render pipeline
	renderer.addRenderPass(std::make_unique<engine::ForwardRenderPass>());

	engine::Input::setMouseTrapped(true);

	std::cout << "Game initialized!\n";
}

void MyGame::update(float deltaTime)
{
	cube->transform.rotate(glm::vec3(1.0f, 0.0f, 1.0f) * (deltaTime * 5.0f));
	pointLightCenter->transform.rotate(glm::vec3(-1.0f, 1.0f, -1.0f) * (deltaTime * 15.0f));
}