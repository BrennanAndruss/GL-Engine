#include "Mesh.h"

Mesh::Mesh(const tinyobj::shape_t& shape)
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	// Allocate space for vertex data in a single buffer
	size_t numVertices = shape.mesh.positions.size() / 3;
	_vertBuf.resize(numVertices * VERTEX_SIZE);

	for (size_t i = 0; i < numVertices; i++)
	{
		size_t vi = VERTEX_SIZE * i;
		_vertBuf[vi++] = shape.mesh.positions[3 * i];
		_vertBuf[vi++] = shape.mesh.positions[3 * i + 1];
		_vertBuf[vi++] = shape.mesh.positions[3 * i + 2];
		_vertBuf[vi++] = shape.mesh.normals[3 * i];
		_vertBuf[vi++] = shape.mesh.normals[3 * i + 1];
		_vertBuf[vi++] = shape.mesh.normals[3 * i + 2];
		if (!shape.mesh.texcoords.empty())
		{
			_vertBuf[vi++] = shape.mesh.texcoords[2 * i];
			_vertBuf[vi++] = shape.mesh.texcoords[2 * i];
		}
	}

	// Bind the vertex data to the buffer
	glBufferData(GL_ARRAY_BUFFER, _vertBuf.size() * sizeof(float), _vertBuf.data(), GL_STATIC_DRAW);

	// Configure the vertex attributes
	size_t stride = VERTEX_SIZE * sizeof(float);
	size_t offset = 0;
	glEnableVertexAttribArray(static_cast<GLuint>(Attrib::Position));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	offset += 3 * sizeof(float);

	glEnableVertexAttribArray(static_cast<GLuint>(Attrib::Normal));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	offset += 3 * sizeof(float);

	if (!shape.mesh.texcoords.empty())
	{
		glEnableVertexAttribArray(static_cast<GLuint>(Attrib::TexCoord));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	}

	// Initialize the element buffer object
	_numIndices = shape.mesh.indices.size();
	glGenBuffers(1, &_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _numIndices * sizeof(unsigned int), shape.mesh.indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	// Initialize the bounding box
	std::vector<float> positions = shape.mesh.positions;
	_bbox.min = glm::vec3(FLT_MAX);
	_bbox.max = glm::vec3(FLT_MIN);

	for (size_t i = 0; i < positions.size(); i += 3)
	{
		_bbox.min.x = std::min(_bbox.min.x, positions[i]);
		_bbox.min.y = std::min(_bbox.min.y, positions[i + 1]);
		_bbox.min.z = std::min(_bbox.min.z, positions[i + 2]);
		_bbox.max.x = std::max(_bbox.max.x, positions[i]);
		_bbox.max.y = std::max(_bbox.max.y, positions[i + 1]);
		_bbox.max.z = std::max(_bbox.max.z, positions[i + 2]);
	}
}

void Mesh::draw() const
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, _numIndices, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}