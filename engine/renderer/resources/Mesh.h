#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include <vector>
#include <cstddef>

namespace engine
{
	static constexpr std::size_t VERTEX_SIZE = 8;

	enum class Attrib
	{
		Position,	// 3
		Normal,		// 3
		TexCoord	// 2
	};

	struct BBox
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	class Mesh
	{
	public:
		Mesh(const tinyobj::shape_t& shape);

		Mesh(const std::vector<glm::vec3>& positions,
			 const std::vector<glm::vec3>& normals,
			 const std::vector<glm::vec2>& texcoords,
			 const std::vector<unsigned int>& indices);

		~Mesh() = default;

		BBox getBBox() const { return _bbox; }
		void draw() const;

	private:
		void setupMesh(const std::vector<glm::vec3>& positions,
					   const std::vector<glm::vec3>& normals,
					   const std::vector<glm::vec2>& texcoords,
					   const std::vector<unsigned int>& indices);

		void computeBBox(const std::vector<glm::vec3>& positions);

	private:
		std::vector<float> _vertBuf;
		std::size_t _numIndices = 0;
		GLuint _vao = 0, _vbo = 0, _ebo = 0;
		BBox _bbox;
	};
}