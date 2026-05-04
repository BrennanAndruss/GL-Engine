#pragma once

#include <vector>

#include "core/Game.h"
#include "core/EventCallbacks.h"
#include "resources/AssetManager.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "systems/FreeCameraController.h"
#include "systems/PlayerController.h"

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
	void setEditorMode(bool editorActive, engine::Scene& scene) override;
	void setEditorSelectionLock(bool locked, engine::Scene& scene) override;

private:
	engine::Object* gem = nullptr;
	engine::Object* cube = nullptr;
	engine::Object* gameplayCameraObject = nullptr;
	engine::Object* editorCameraObject = nullptr;
	engine::Object* pointLightCenter = nullptr;
	Handle<engine::Mesh> cubeMesh;
	PlayerController* gameplayController = nullptr;
	FreeCameraController* editorController = nullptr;
	bool editorModeActive = false;
	bool editorCameraLocked = false;
	std::vector<engine::Object*> objects;

	void refreshEditorCameraState(engine::Scene& scene);
};