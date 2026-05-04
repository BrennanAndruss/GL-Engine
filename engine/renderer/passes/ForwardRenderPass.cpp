#include "renderer/passes/ForwardRenderPass.h"

#include <algorithm>
#include <glad/glad.h>
#include <string>
#include "renderer/resources/Shader.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "renderer/resources/Texture.h"
#include "resources/AssetManager.h"
#include "scene/components/Component.h"
#include "scene/components/Animator.h"
#include "scene/components/MeshRenderer.h"

namespace engine
{
	static constexpr std::size_t MAX_SHADER_BONES = 100;

	ForwardRenderPass::ForwardRenderPass() = default;

	void ForwardRenderPass::execute(const Scene& scene, const AssetManager& assets)
	{
		for (const auto& object : scene.getObjects())
		{
			auto* meshRenderer = object->getComponent<MeshRenderer>();
			if (!meshRenderer) continue;

			auto* mesh = assets.getMesh(meshRenderer->mesh);
			auto* mat = assets.getMaterial(meshRenderer->material);
			if (!mesh || !mat) continue;
			auto* shader = assets.getShader(mat->shader);
			if (!shader) continue;

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
			shader->setInt("isSkinned", 0);

			if (mat->difTex.valid())
			{
				if (auto* difTex = assets.getTexture(mat->difTex))
				{

					difTex->bind(shader->getUniform("mat.difTex"));
					shader->setInt("mat.hasDifTex", 1);
				
				}
			}
	

			if (mat->specTex.valid())
			{
				if (auto* specTex = assets.getTexture(mat->specTex))
				{
					specTex->bind(shader->getUniform("mat.specTex"));
					shader->setInt("mat.hasSpecTex", 1);
				}
					
			}

			if (mesh->isSkinned())
			{
				if (auto* animator = object->getComponent<Animator>())
				{
					const auto& boneMatrices = animator->getBoneMatrices();
					const int numBones = static_cast<int>(std::min(boneMatrices.size(), MAX_SHADER_BONES));
					shader->setInt("isSkinned", 1);
					shader->setInt("numBones", numBones);
					for (int i = 0; i < numBones; ++i)
					{
						shader->setMat4("bones[" + std::to_string(i) + "]", boneMatrices[static_cast<std::size_t>(i)]);
					}
				}
			}

			mesh->draw();
			shader->unbind();
		}
	}
}