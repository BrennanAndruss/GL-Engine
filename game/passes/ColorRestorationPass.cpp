#include "passes/ColorRestorationPass.h"

#include "renderer/FullscreenQuad.h"

ColorRestorationPass::ColorRestorationPass(int width, int height, Handle<engine::Shader> shader) : 
	_shader(shader),
	_framebuffer(width, height, {{ engine::AttachmentFormat::RGBA8 }}) {}

void ColorRestorationPass::resize(int width, int height)
{
	_framebuffer.resize(width, height);
}

void ColorRestorationPass::execute(const engine::Scene& scene, 
	const engine::AssetManager& assets, engine::RenderContext& ctx)
{
	GLuint inputTex = ctx.getBuffer(engine::BufferNames::SceneColor);

	_framebuffer.bind();
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	auto* shader = assets.getShader(_shader);
	
	// Set shader uniforms
	shader->bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTex);
	shader->setInt("sceneTex", 0);
	shader->setFloat("cyan", cyan);
	shader->setFloat("magenta", magenta);
	shader->setFloat("yellow", yellow);
	shader->setFloat("key", key);

	engine::FullscreenQuad::getInstance().draw();

	shader->unbind();
	glEnable(GL_DEPTH_TEST);
	_framebuffer.unbind();

	// Register outputs with render context
	ctx.sceneFramebuffer = &_framebuffer;
	ctx.setBuffer(engine::BufferNames::SceneColor, 
		_framebuffer.getAttachment(engine::AttachmentFormat::RGBA8));
}