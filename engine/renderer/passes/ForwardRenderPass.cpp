#include "renderer/passes/ForwardRenderPass.h"

#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <vector>
#include "core/Input.h"
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
	static std::unique_ptr<Shader> g_debugLineShader;

	ForwardRenderPass::ForwardRenderPass() = default;

	void ForwardRenderPass::execute(const Scene& scene, const AssetManager& assets)
	{
		static bool showSkeletonVisualizer = true;
		if (Input::isKeyPressed(GLFW_KEY_F3))
		{
			showSkeletonVisualizer = !showSkeletonVisualizer;
			std::cout << "[ForwardRenderPass] Skeleton visualizer "
				<< (showSkeletonVisualizer ? "enabled" : "disabled") << " (F3)\n";
		}

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

				// Debug: draw skeleton lines if this object has an animator
				if (showSkeletonVisualizer && mesh->isSkinned())
				{
					if (auto* animator = object->getComponent<Animator>())
					{
						if (animator->hasPose())
						{
							if (auto* skel = assets.getSkeleton(animator->skeleton))
							{
								const auto& globals = animator->getGlobalMatrices();
								const glm::mat4 objectWorld = object->transform.getWorldMatrix();
								std::vector<glm::vec3> lineVerts;
								lineVerts.reserve(skel->nodes.size() * 2);

								for (const auto& node : skel->nodes)
								{
									if (!node.isBone || node.parentIndex < 0) continue;
									const int parentIdx = skel->nodes[node.parentIndex].boneIndex;
									const int childIdx = node.boneIndex;
									if (parentIdx < 0 || childIdx < 0) continue;
									if (static_cast<std::size_t>(parentIdx) >= globals.size() || static_cast<std::size_t>(childIdx) >= globals.size()) continue;

									glm::vec3 parentPos = glm::vec3(objectWorld * globals[static_cast<std::size_t>(parentIdx)][3]);
									glm::vec3 childPos = glm::vec3(objectWorld * globals[static_cast<std::size_t>(childIdx)][3]);
									lineVerts.push_back(parentPos);
									lineVerts.push_back(childPos);
								}

								if (!lineVerts.empty())
								{
									// Create a simple line shader on first use
									if (!g_debugLineShader)
									{
										const std::string vsrc = "#version 410 core\nlayout(location=0) in vec3 vertPos;\nlayout(std140) uniform CameraData{ mat4 view; mat4 projection; vec4 cameraPos; };\nuniform mat4 model;\nvoid main(){ gl_Position = projection * view * model * vec4(vertPos, 1.0); }";
										const std::string fsrc = "#version 410 core\nout vec4 fragColor;\nvoid main(){ fragColor = vec4(1.0, 0.0, 0.0, 1.0); }";
										g_debugLineShader = std::make_unique<Shader>(vsrc, fsrc);
									}

									GLuint vao = 0, vbo = 0;
									glGenVertexArrays(1, &vao);
									glGenBuffers(1, &vbo);
									glBindVertexArray(vao);
									glBindBuffer(GL_ARRAY_BUFFER, vbo);
									glBufferData(GL_ARRAY_BUFFER, lineVerts.size() * sizeof(glm::vec3), lineVerts.data(), GL_DYNAMIC_DRAW);
									glEnableVertexAttribArray(0);
									glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

									g_debugLineShader->bind();
									g_debugLineShader->setMat4("model", glm::mat4(1.0f));
									glDisable(GL_DEPTH_TEST);
									glLineWidth(2.0f);
									glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVerts.size()));
									glEnable(GL_DEPTH_TEST);
									g_debugLineShader->unbind();

									glDisableVertexAttribArray(0);
									glBindBuffer(GL_ARRAY_BUFFER, 0);
									glBindVertexArray(0);
									glDeleteBuffers(1, &vbo);
									glDeleteVertexArrays(1, &vao);
								}
							}
						}
					}
				}
		}
	}
}