#include "renderer/Renderer.h"

#include <glad/glad.h>
#include <cassert>
#include <iostream>

namespace engine
{
	Renderer::Renderer(int width, int height) : 
		_width(width), 
		_height(height), 
		_cameraUBO(sizeof(CameraData), static_cast<GLuint>(UBOBindings::Camera)),
		_lightsUBO(MAX_LIGHTS * sizeof(LightData), static_cast<GLuint>(UBOBindings::Light)) {}

	void Renderer::addRenderPass(std::unique_ptr<RenderPass> pass)
	{
		assert(pass != nullptr && "Null render pass.");
		_renderPasses.push_back(std::move(pass));
	}

	void Renderer::resize(int width, int height)
	{
		_width = width;
		_height = height;
	}

	void Renderer::render(const Scene& scene, const AssetManager& assets)
	{
		// Clear the previous frame
		glViewport(0, 0, _width, _height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update uniform buffer objects
		auto* camera = scene.getCamera();
		assert(camera != nullptr && "Scene has no camera.");
		_cameraUBO.update(&camera->getCameraData(), sizeof(CameraData));

		const auto& lights = scene.getLights();
		int numLights = std::min((int)lights.size(), MAX_LIGHTS);
		for (int i = 0; i < lights.size(); i++)
		{
			LightData data = lights[i]->getLightData();
			_lightsUBO.update(&data, sizeof(LightData), i * sizeof(LightData));
		}

		for (const auto& pass : _renderPasses)
		{
			pass->execute(scene, assets);
		}
	}
}
