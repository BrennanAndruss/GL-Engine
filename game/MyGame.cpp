#include "MyGame.h"

#include <iostream>
#include <memory>
#include <glm/glm.hpp>

#include "core/Input.h"
#include "resources/AssetManager.h"
#include "resources/Heightmap.h"
#include "resources/SkeletalAnimation.h"

#include "renderer/resources/Shader.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "renderer/resources/Cubemap.h"

#include "renderer/passes/SkyboxRenderPass.h"
#include "renderer/passes/ForwardRenderPass.h"

#include "scene/components/Components.h"

#include "systems/PlayerController.h"
#include "systems/Collectable.h"

void MyGame::init(
	engine::AssetManager& assets,
	engine::Renderer& renderer,
	engine::Scene& scene,
	const engine::AppConfig& config)
{
	std::cout << "Loading shaders...\n";
	Handle<engine::Shader> shader = assets.getShaderHandle("EngineForward");
	Handle<engine::Shader> skinnedShader = shader;

	std::cout << "Loading textures...\n";

	Handle<engine::Heightmap> terrainHeightmap = assets.loadHeightmap(
		"terrainHM",
		"textures/heightmaps/HM_Unity02.png",
		25.0f
	);

	auto* heightmap = assets.getHeightmap(terrainHeightmap);

	Handle<engine::Cubemap> skyboxCubemap = assets.loadCubemap(
		"daySkybox",
		{
			"textures/px.png",
			"textures/nx.png",
			"textures/py.png",
			"textures/ny.png",
			"textures/pz.png",
			"textures/nz.png"
		}
	);

	Handle<engine::Texture> gemDiffuseTex = assets.loadTexture(
		"gemDiffuse",
		"textures/yellow_gem_texture.png",
		true
	);

	Handle<engine::Texture> charBaseTex = assets.loadTexture(
		"charBase",
		"textures/char_Base_color.png",
		true
	);

	std::cout << "Loading models...\n";

	Handle<engine::Mesh> gemMesh;

	try
	{
		gemMesh = assets.loadMeshAssimp("gem", "models/gem_model.fbx");
		std::cout << "Loaded Assimp mesh: models/gem_model.fbx\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "Assimp gem load failed, falling back to cube.obj: " << e.what() << "\n";
		gemMesh = assets.loadMesh("gem", "models/cube.obj");
	}

	Handle<engine::Mesh> playerMesh;
	Handle<engine::Skeleton> playerSkeleton;
	Handle<engine::AnimationClip> playerClip;

	try
	{
		playerMesh = assets.loadMeshAssimp(
			"playerMesh",
			"models/Sprint.fbx"
		);

		playerSkeleton = assets.loadSkeletonAssimp(
			"playerSkeleton",
			"models/Sprint.fbx"
		);

		playerClip = assets.loadAnimationClipAssimp(
			"playerSprintAnimation",
			"models/Sprint.fbx"
		);

		const auto* skel = assets.getSkeleton(playerSkeleton);
		const auto* clip = assets.getAnimationClip(playerClip);

		if (skel && clip)
		{
			std::cout
				<< "[Player] Skeleton loaded: "
				<< skel->nodes.size()
				<< " nodes, "
				<< skel->boneCount()
				<< " bones\n";

			std::cout
				<< "[Player] Animation loaded: \""
				<< clip->name
				<< "\" duration="
				<< clip->durationTicks
				<< " ticks @ "
				<< clip->ticksPerSecond
				<< " TPS, tracks="
				<< clip->tracks.size()
				<< "\n";
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Player animated model load failed: " << e.what() << "\n";
	}

	int planeRes = heightmap->getWidth() / 2 - 1;
	float planeLen = 100.0f;

	Handle<engine::Mesh> terrainMesh = assets.createHeightmapMesh(
		"terrain",
		terrainHeightmap,
		planeRes,
		planeLen
	);

	std::cout << "Loading materials...\n";

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
	mat->ambient = glm::vec3(0.113f, 0.152f, 0.081f);
	mat->diffuse = glm::vec3(0.565f, 0.761f, 0.404f);
	mat->specular = glm::vec3(0.5f);
	mat->shininess = 32.0f;

	Handle<engine::Material> redMat = assets.loadMaterial("redMat");
	mat = assets.getMaterial(redMat);
	mat->shader = shader;
	mat->ambient = glm::vec3(0.2f, 0.0f, 0.0f);
	mat->diffuse = glm::vec3(0.8f, 0.0f, 0.0f);
	mat->specular = glm::vec3(1.0f);
	mat->shininess = 32.0f;

	Handle<engine::Material> gemMat = assets.loadMaterial("gemMat");
	mat = assets.getMaterial(gemMat);
	mat->shader = shader;
	mat->ambient = glm::vec3(0.2f);
	mat->diffuse = glm::vec3(0.8f);
	mat->specular = glm::vec3(1.0f);
	mat->shininess = 64.0f;
	mat->difTex = gemDiffuseTex;
	mat->specTex = gemDiffuseTex;

	Handle<engine::Material> playerMat = assets.loadMaterial("playerMat");
	mat = assets.getMaterial(playerMat);
	mat->shader = skinnedShader;
	mat->ambient = glm::vec3(0.2f);
	mat->diffuse = glm::vec3(0.8f);
	mat->specular = glm::vec3(1.0f);
	mat->shininess = 20.0f;
	mat->difTex = charBaseTex;
	mat->specTex = charBaseTex;

	// Directional light
	{
		auto& obj = scene.createObject("DirLight");
		auto& dirLight = obj.addComponent<engine::DirectionalLight>();

		obj.transform.lookAt(glm::vec3(0.8f, -1.0f, 0.6f));

		dirLight.setColor(glm::vec3(1.0f));
		dirLight.setIntensity(1.0f);
	}

	// Point lights
	pointLightCenter = &scene.createObject("PointLightCenter");
	pointLightCenter->transform.setPosition(glm::vec3(0.0f, 3.5f, -5.0f));

	{
		auto& obj = scene.createObject("PointLight1");
		obj.transform.setParent(&pointLightCenter->transform);
		obj.transform.setPosition(glm::vec3(2.5f, 0.0f, 0.0f));

		auto& light = obj.addComponent<engine::PointLight>();
		light.setRange(10.0f);
		light.setColor(glm::vec3(1.0f, 0.0f, 0.0f));
		light.setIntensity(1.0f);
	}

	{
		auto& obj = scene.createObject("PointLight2");
		obj.transform.setParent(&pointLightCenter->transform);
		obj.transform.setPosition(glm::vec3(0.0f, -2.5f, 0.0f));

		auto& light = obj.addComponent<engine::PointLight>();
		light.setRange(10.0f);
		light.setColor(glm::vec3(0.0f, 0.0f, 1.0f));
		light.setIntensity(1.0f);
	}

	{
		auto& obj = scene.createObject("PointLight3");
		obj.transform.setParent(&pointLightCenter->transform);
		obj.transform.setPosition(glm::vec3(0.0f, 0.0f, 2.5f));

		auto& light = obj.addComponent<engine::PointLight>();
		light.setRange(10.0f);
		light.setColor(glm::vec3(0.0f, 1.0f, 0.0f));
		light.setIntensity(1.0f);
	}

	// Gem
	{
		gem = &scene.createObject("Gem");
		gem->transform.setPosition(glm::vec3(0.0f, 4.0f, 0.0f));
		gem->transform.setScale(glm::vec3(0.5f));

		auto& meshRenderer = gem->addComponent<engine::MeshRenderer>();
		meshRenderer.mesh = gemMesh;
		meshRenderer.material = gemMat;
	}

	// Terrain
	{
		auto& terrain = scene.createObject("Floor");

		auto& collider = terrain.addComponent<engine::HeightmapCollider>();
		collider.heightmap = heightmap;
		collider.planeLen = planeLen;

		auto& meshRenderer = terrain.addComponent<engine::MeshRenderer>();
		meshRenderer.mesh = terrainMesh;
		meshRenderer.material = grassMat;
	}

	// Player + animated model + third-person camera
	{
		auto& player = scene.createObject("Player");
		cube = &player;

		player.transform.setPosition(glm::vec3(5.0f, 10.0f, 0.0f));
		player.transform.setScale(glm::vec3(0.2f));

		auto& meshRenderer = player.addComponent<engine::MeshRenderer>();
		meshRenderer.mesh = playerMesh;
		meshRenderer.material = playerMat;

		auto& animator = player.addComponent<engine::Animator>();
		animator.assets = &assets;
		animator.skeleton = playerSkeleton;
		animator.clip = playerClip;
		animator.playbackSpeed = 1.0f;
		animator.loop = true;

		auto& charController = player.addComponent<engine::CharacterController>();
		charController.height = 1.0f;

		auto& playerController = player.addComponent<PlayerController>();
		gameplayController = &playerController;

		playerController.moveSpeed = 10.0f;
		playerController.eyeHeight = 0.5f;
		playerController.cameraDistance = 7.0f;

		auto& camObj = scene.createObject("MainCamera");
		gameplayCameraObject = &camObj;

		float aspect =
			static_cast<float>(config.width) /
			static_cast<float>(config.height);

		auto& camera = camObj.addComponent<engine::Camera>(
			45.0f,
			aspect,
			0.1f,
			100.0f
		);

		scene.setMainCamera(&camera);

		playerController.cameraTransform = &camObj.transform;
	}

	{
		auto& camObj = scene.createObject("EditorCamera");
		editorCameraObject = &camObj;

		const glm::vec3 playerPos = cube->transform.getPosition();
		camObj.transform.setPosition(playerPos + glm::vec3(0.0f, 3.0f, 8.0f));
		camObj.transform.lookAt(playerPos + glm::vec3(0.0f, 1.0f, 0.0f));

		float aspect =
			static_cast<float>(config.width) /
			static_cast<float>(config.height);

		auto& camera = camObj.addComponent<engine::Camera>(
			45.0f,
			aspect,
			0.1f,
			200.0f
		);
		(void)camera;

		editorController = &camObj.addComponent<FreeCameraController>();
		editorController->enabled = false;
	}

	scene.setSkybox(skyboxCubemap);

	// Collectables
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

	// renderer.addRenderPass(std::make_unique<engine::SkyboxRenderPass>(skyboxShader));
	// renderer.addRenderPass(std::make_unique<engine::ForwardRenderPass>(shader));

	engine::Input::setMouseTrapped(true);

	std::cout << "Game initialized!\n";
}

void MyGame::update(float deltaTime)
{
	if (gem)
	{
		gem->transform.rotate(glm::vec3(1.0f, 0.0f, 1.0f) * (deltaTime * 5.0f));
	}
}

void MyGame::setEditorMode(bool editorActive, engine::Scene& scene)
{
	const bool wasEditorMode = editorModeActive;
	editorModeActive = editorActive;

	if (!wasEditorMode && editorModeActive && gameplayCameraObject && editorCameraObject)
	{
		editorCameraObject->transform.setPosition(gameplayCameraObject->transform.getPosition());
		editorCameraObject->transform.setRotation(gameplayCameraObject->transform.getRotation());
		editorController->syncFromTransform(gameplayCameraObject->transform);
	}
	else if (wasEditorMode && !editorModeActive && gameplayCameraObject && editorCameraObject)
	{
		gameplayCameraObject->transform.setPosition(editorCameraObject->transform.getPosition());
		gameplayCameraObject->transform.setRotation(editorCameraObject->transform.getRotation());
	}

	refreshEditorCameraState(scene);
}

void MyGame::setEditorSelectionLock(bool locked, engine::Scene& scene)
{
	(void)locked;
	editorCameraLocked = false;
	refreshEditorCameraState(scene);
}

void MyGame::refreshEditorCameraState(engine::Scene& scene)
{
	if (!gameplayCameraObject || !editorCameraObject || !gameplayController || !editorController)
	{
		return;
	}

	gameplayController->enabled = !editorModeActive;
	editorController->enabled = editorModeActive;

	if (editorModeActive)
	{
		scene.setMainCamera(editorCameraObject->getComponent<engine::Camera>());
	}
	else
	{
		scene.setMainCamera(gameplayCameraObject->getComponent<engine::Camera>());
	}
}