#include "renderer/passes/BlitPass.h"

#include <glad/glad.h>
#include "renderer/Framebuffer.h"
#include "renderer/RenderContext.h"
#include "resources/AssetManager.h"
#include "scene/Scene.h"

namespace engine
{
	void BlitPass::execute(const Scene& scene,
		const AssetManager& assets, RenderContext& ctx)
	{
		GLuint sceneFbo = ctx.sceneFramebuffer->getFboId();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		
		// Blit scene FBO to screen
		glBlitFramebuffer(
			0, 0, ctx.width, ctx.height,
			0, 0, ctx.width, ctx.height,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR
		);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}