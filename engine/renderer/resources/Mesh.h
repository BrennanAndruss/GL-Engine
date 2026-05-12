#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <cstddef>

namespace engine
{
	static constexpr std::size_t VERTEX_SIZE = 8;
	static constexpr std::size_t MAX_BONE_INFLUENCES = 4;

	enum class Attrib
	{
		Position,	// 3
		Normal,		// 3
		TexCoord,	// 2
		BoneIds,	// ivec4
		BoneWeights	// vec4, all weights should sum to EXACTLY 1.0f 
	};

	struct BBox
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	class Mesh
	{
	public:
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
		// Overload for skinned meshes that takes the bone data.
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
		GLuint _vao = 0, _vbo = 0, _ebo = 0;
		GLuint _boneIdVbo = 0, _boneWeightVbo = 0;
		bool _isSkinned = false;
		BBox _bbox;
	};
}