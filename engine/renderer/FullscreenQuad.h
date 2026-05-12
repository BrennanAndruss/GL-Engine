#pragma once

#include <glad/glad.h>

namespace engine
{
	// Fullscreen quad shared by all post-process passes
	class FullscreenQuad
	{
	public:
		static FullscreenQuad& getInstance()
		{
			static FullscreenQuad instance;
			return instance;
		}

		FullscreenQuad(const FullscreenQuad&) = delete;
		FullscreenQuad& operator=(const FullscreenQuad&) = delete;

		void draw() const;

	private:
		FullscreenQuad();
		~FullscreenQuad();

		GLuint _vao = 0, _vbo = 0;
	};
}