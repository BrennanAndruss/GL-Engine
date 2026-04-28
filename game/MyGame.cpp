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
	Handle<engine::Shader> shader = assets.loadShader("simple", "shaders/simple.vert", "shaders/simple.frag");

	std::cout << "Loading textures...\n";
	Handle<engine::Heightmap> terrainHeightmap = assets.loadHeightmap("terrainHM", "textures/heightmaps/HM_Unity02.png", 25.0f);
	auto* heightmap = assets.getHeightmap(terrainHeightmap);

    //loading textures
	Handle<engine::Texture> gemDiffuseTex = assets.loadTexture("gemDiffuse", "textures/yellow_gem_texture.png", true);

	std::cout << "Loading models...\n";
	Handle<engine::Mesh> gemMesh;
	try
	{
		gemMesh = assets.loadMeshAssimp("gem", "models/gem_model.fbx");
		std::cout << "Loaded Assimp mesh: models/gem_model.fbx\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "Assimp load failed, falling back to OBJ: " << e.what() << "\n";
		gemMesh = assets.loadMesh("gem", "models/cube.obj");
	}

	int planeRes = heightmap->getWidth() / 2 - 1; // 256x256 vertices (half-resolution)
	float planeLen = 100.0f;
	Handle<engine::Mesh> terrainMesh = assets.createHeightmapMesh("terrain", terrainHeightmap, planeRes, planeLen);
	
	std::cout << "Loading materials...\n";
	//make plane
	Handle<engine::Material> grassMat = assets.loadMaterial("grassMat");
	auto* mat = assets.getMaterial(grassMat);
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

	Handle<engine::Material> gemMat = assets.loadMaterial("gemMat");
	mat = assets.getMaterial(gemMat);
	mat->shader = shader;
	mat->ambient = glm::vec3(0.2f);
	mat->diffuse = glm::vec3(0.8f);
	mat->specular = glm::vec3(1.0f);
	mat->shininess = 64.0f;
	mat->difTex = gemDiffuseTex;
	mat->specTex = gemDiffuseTex; //using diffuse texture as specular for a shiny effect

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
		gem = &scene.createObject("Gem");
		gem->transform.setPosition(glm::vec3(0.0f, 4.0f, 0.0f));
		gem->transform.setScale(glm::vec3(0.5f));

		auto& meshRenderer = gem->addComponent<engine::MeshRenderer>();
		meshRenderer.mesh = gemMesh;
		meshRenderer.material = gemMat;

		//cube->addComponent<engine::BoxCollider>();
		//cube->addComponent<engine::RigidBody>();
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
		mr.mesh = gemMesh;
		mr.material = redMat;

		auto& collider = obj.addComponent<engine::BoxCollider>();
		collider.size = obj.transform.getScale();
		collider.isTrigger = true;

		auto& collectable = obj.addComponent<Collectable>();
		collectable.defaultMat = redMat; 
		collectable.collectedMat = gemMat;
	}

	// Configure render pipeline
	renderer.addRenderPass(std::make_unique<engine::ForwardRenderPass>());

	engine::Input::setMouseTrapped(true);

	std::cout << "Game initialized!\n";
}

void MyGame::update(float deltaTime)
{
	gem->transform.rotate(glm::vec3(1.0f, 0.0f, 1.0f) * (deltaTime * 5.0f));
}