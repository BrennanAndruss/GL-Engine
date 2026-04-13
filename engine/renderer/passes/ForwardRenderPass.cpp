#include "renderer/passes/ForwardRenderPass.h"

#include <glad/glad.h>
#include "renderer/resources/Shader.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "resources/AssetManager.h"
#include "scene/components/Component.h"
#include "scene/components/MeshRenderer.h"

namespace engine
{
	ForwardRenderPass::ForwardRenderPass() = default;

	void ForwardRenderPass::execute(const Scene& scene, const AssetManager& assets)
	{
		for (const auto& object : scene.getObjects())
		{
			auto* meshRenderer = object->getComponent<MeshRenderer>();
			if (!meshRenderer) continue;

			auto* mesh = assets.getMesh(meshRenderer->meshId);
			auto* mat = assets.getMaterial(meshRenderer->materialId);
			auto* shader = assets.getShader(mat->shaderId);
			if (!mesh || !mat || !shader) continue;

			// Set simple shader uniforms
			shader->bind();
			shader->setMat4("model", object->transform.getMatrix());
			shader->setVec3("mat.ambient", mat->ambient);
			shader->setVec3("mat.diffuse", mat->diffuse);
			shader->setVec3("mat.specular", mat->specular);
			shader->setFloat("mat.shininess", mat->shininess);
			shader->setInt("numLights", scene.getLights().size());

			mesh->draw();
			shader->unbind();
		}
	}
}