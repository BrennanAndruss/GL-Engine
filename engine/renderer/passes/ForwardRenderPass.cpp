#include "renderer/passes/ForwardRenderPass.h"

#include <algorithm>
#include <glad/glad.h>

#include "renderer/resources/Shader.h"
#include "renderer/resources/Mesh.h"
#include "renderer/resources/Material.h"
#include "renderer/resources/Texture.h"

#include "resources/AssetManager.h"

#include "scene/components/Component.h"
#include "scene/components/MeshRenderer.h"
#include "scene/components/Animator.h"

namespace engine
{
	ForwardRenderPass::ForwardRenderPass(Handle<Shader> shader)
		: RenderPass(shader)
	{
	}

	void ForwardRenderPass::execute(const Scene& scene, const AssetManager& assets)
	{
		for (const auto& object : scene.getObjects())
		{
			auto* meshRenderer = object->getComponent<MeshRenderer>();
			if (!meshRenderer)
			{
				continue;
			}

			auto* mesh = assets.getMesh(meshRenderer->mesh);
			auto* mat = assets.getMaterial(meshRenderer->material);

			if (!mesh || !mat)
			{
				continue;
			}

			auto* shader = assets.getShader(mat->shader);
			if (!shader)
			{
				continue;
			}

			shader->bind();

			shader->setMat4("model", object->transform.getWorldMatrix());

			shader->setVec3("mat.ambient", mat->ambient);
			shader->setVec3("mat.diffuse", mat->diffuse);
			shader->setVec3("mat.specular", mat->specular);
			shader->setFloat("mat.shininess", mat->shininess);

			shader->setInt("numLights", static_cast<int>(scene.getLights().size()));

			shader->setInt("isSkinned", 0);

			if (mesh->isSkinned())
			{
				auto* animator = object->getComponent<Animator>();

				if (animator && animator->hasPose())
				{
					shader->setInt("isSkinned", 1);

					const auto& boneMatrices = animator->getBoneMatrices();

					const int count = std::min(
						static_cast<int>(boneMatrices.size()),
						100
					);

					for (int i = 0; i < count; ++i)
					{
						shader->setMat4(
							"bones[" + std::to_string(i) + "]",
							boneMatrices[static_cast<std::size_t>(i)]
						);
					}
				}
			}

			auto* difTex = assets.getTexture(mat->difTex);
			if (difTex)
			{
				difTex->bind(shader->getUniform("mat.difTex"));
			}

			auto* specTex = assets.getTexture(mat->specTex);
			if (specTex)
			{
				specTex->bind(shader->getUniform("mat.specTex"));
			}

			mesh->draw();

			if (difTex)
			{
				difTex->unbind();
			}

			if (specTex)
			{
				specTex->unbind();
			}

			shader->unbind();
		}
	}
}