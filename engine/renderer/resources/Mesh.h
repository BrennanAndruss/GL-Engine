#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>
#include <vector>

static constexpr size_t VERTEX_SIZE = 8;

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
	~Mesh() = default;

	BBox getBBox() const { return _bbox; }
	void draw() const;

private:
	std::vector<float> _vertBuf;
	size_t _numIndices;
	GLuint _vao, _vbo, _ebo;
	BBox _bbox;
};