#pragma once

#include "scene/Scene.h"
#include "renderer/passes/RenderPass.h"
#include "renderer/resources/Shader.h"
#include "renderer/RenderContext.h"
#include "renderer/Framebuffer.h"
#include "resources/AssetManager.h"

class ColorRestorationPass : public engine::RenderPass
{
public:
	ColorRestorationPass(int width, int height, Handle<engine::Shader> shader);

	void resize(int width, int height) override;
	void execute(const engine::Scene& scene, const engine::AssetManager& assets,
		engine::RenderContext& ctx) override;

	float cyan = 0.0f, magenta = 0.0f, yellow = 0.0f, key = 0.0f;

private:
	Handle<engine::Shader> _shader;
	engine::Framebuffer _framebuffer;
};