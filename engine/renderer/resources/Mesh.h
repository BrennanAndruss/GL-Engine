#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

#include <cstddef>
#include <vector>

#include "resources/SkeletalAnimation.h"

namespace engine
{
	static constexpr std::size_t VERTEX_SIZE = 8;

	enum class Attrib
	{
		Position = 0,   // vec3
		Normal = 1,     // vec3
		TexCoord = 2,   // vec2
		BoneIds = 3,    // uvec4
		BoneWeights = 4 // vec4
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

		Mesh(const std::vector<glm::vec3>& positions,
			 const std::vector<glm::vec3>& normals,
			 const std::vector<glm::vec2>& texcoords,
			 const std::vector<unsigned int>& indices,
			 const std::vector<glm::uvec4>& boneIds,
			 const std::vector<glm::vec4>& boneWeights);

		~Mesh() = default;

		BBox getBBox() const { return _bbox; }

		bool isSkinned() const { return _isSkinned; }

		void draw() const;

	private:
		void setupMesh(const std::vector<glm::vec3>& positions,
					   const std::vector<glm::vec3>& normals,
					   const std::vector<glm::vec2>& texcoords,
					   const std::vector<unsigned int>& indices);

		void setupMesh(const std::vector<glm::vec3>& positions,
					   const std::vector<glm::vec3>& normals,
					   const std::vector<glm::vec2>& texcoords,
					   const std::vector<unsigned int>& indices,
					   const std::vector<glm::uvec4>& boneIds,
					   const std::vector<glm::vec4>& boneWeights);

		void computeBBox(const std::vector<glm::vec3>& positions);

	private:
		std::vector<float> _vertBuf;
		std::vector<glm::uvec4> _boneIdBuf;
		std::vector<glm::vec4> _boneWeightBuf;

		std::size_t _numIndices = 0;

		GLuint _vao = 0;
		GLuint _vbo = 0;
		GLuint _ebo = 0;
		GLuint _boneIdVbo = 0;
		GLuint _boneWeightVbo = 0;

		bool _isSkinned = false;

		BBox _bbox;
	};
}