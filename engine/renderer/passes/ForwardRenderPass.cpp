#include "renderer/passes/ForwardRenderPass.h"

#include <glad/glad.h>
#include "renderer/RenderContext.h"
#include "renderer/resources/Shader.h"
#include "renderer/resources/Cubemap.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "renderer/resources/Texture.h"
#include "resources/AssetManager.h"

#include "scene/Scene.h"
#include "scene/components/Component.h"
#include "scene/components/MeshRenderer.h"

namespace engine
{
	static const float SKYBOX_VERTICES[] =
	{
		-1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
	};

	ForwardRenderPass::ForwardRenderPass(int width, int height, Handle<Shader> shader) :
		_shader(shader),
		_framebuffer(width, height, {
			{ AttachmentFormat::RGBA8 },
			{ AttachmentFormat::Depth24 }
			}) 
	{
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vbo);

		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(SKYBOX_VERTICES), SKYBOX_VERTICES, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindVertexArray(0);
	}

	ForwardRenderPass::~ForwardRenderPass()
	{
		if (_vbo != 0) glDeleteBuffers(1, &_vbo);
		if (_vao != 0) glDeleteVertexArrays(1, &_vao);
	}

	void ForwardRenderPass::resize(int width, int height)
	{
		_framebuffer.resize(width, height);
	}

	void ForwardRenderPass::execute(const Scene& scene, 
		const AssetManager& assets, RenderContext& ctx)
	{
		_framebuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw opaque geometry
		auto* shader = assets.getShader(_shader);
		for (const auto& object : scene.getObjects())
		{
			auto* meshRenderer = object->getComponent<MeshRenderer>();
			if (!meshRenderer) continue;

			auto* mesh = assets.getMesh(meshRenderer->mesh);
			auto* mat = assets.getMaterial(meshRenderer->material);
			if (!mesh || !mat) continue;

			// Skip materials using other shaders (non-default)
			if (_shader.index != mat->shader.index) continue;

			// Set shader uniforms
			shader->bind();
			shader->setMat4("model", object->transform.getWorldMatrix());
			shader->setVec3("mat.ambient", mat->ambient);
			shader->setVec3("mat.diffuse", mat->diffuse);
			shader->setVec3("mat.specular", mat->specular);
			shader->setFloat("mat.shininess", mat->shininess);
			shader->setInt("numLights", scene.getLights().size());

			// Bind textures
			auto* difTex = assets.getTexture(mat->difTex);
			difTex->bind(shader->getUniform("mat.difTex"));
	
			auto* specTex = assets.getTexture(mat->specTex);
			specTex->bind(shader->getUniform("mat.specTex"));

			mesh->draw();
			shader->unbind();
			difTex->unbind();
			specTex->unbind();
		}

		_framebuffer.unbind();
		
		// Register outputs with render context
		ctx.setBuffer(BufferNames::SceneColor, _framebuffer.getAttachment(AttachmentFormat::RGBA8));
		ctx.setBuffer(BufferNames::SceneDepth, _framebuffer.getAttachment(AttachmentFormat::Depth24));
		ctx.sceneFramebuffer = &_framebuffer;
	}
}