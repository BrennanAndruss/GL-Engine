#include "renderer/passes/ForwardRenderPass.h"

#include <glad/glad.h>
#include "renderer/resources/Shader.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "renderer/resources/Texture.h"
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

			auto* mesh = assets.getMesh(meshRenderer->mesh);
			auto* mat = assets.getMaterial(meshRenderer->material);
			auto* shader = assets.getShader(mat->shader);
			if (!mesh || !mat || !shader) continue;

			// Set simple shader uniforms
			shader->bind();
			shader->setMat4("model", object->transform.getWorldMatrix());
			shader->setVec3("mat.ambient", mat->ambient);
			shader->setVec3("mat.diffuse", mat->diffuse);
			shader->setVec3("mat.specular", mat->specular);
			shader->setFloat("mat.shininess", mat->shininess);
			shader->setInt("numLights", scene.getLights().size());
			shader->setInt("mat.hasDifTex", 0);
			shader->setInt("mat.hasSpecTex", 0);

			if(mat->difTex.valid()){
				if (auto* difTex = assets.getTexture(mat->difTex)){

					difTex->bind(shader->getUniform("mat.difTex"));
					shader->setInt("mat.hasDifTex", 1);
				
				}
			}
	

			if(mat->specTex.valid()){
				if (auto* specTex = assets.getTexture(mat->specTex)){
					specTex->bind(shader->getUniform("mat.specTex"));
					shader->setInt("mat.hasSpecTex", 1);
				}
					
			}

			mesh->draw();
			shader->unbind();
		}
	}
}