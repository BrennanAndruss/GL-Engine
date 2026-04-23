#include "renderer/resources/Mesh.h"

#include <algorithm>
#include <cfloat>

namespace engine
{
	Mesh::Mesh(const tinyobj::shape_t& shape)
	{
		std::size_t numVertices = shape.mesh.positions.size() / 3;

		std::vector<glm::vec3> positions(numVertices);
		std::vector<glm::vec3> normals(numVertices, glm::vec3(0.0f, 1.0f, 0.0f));
		std::vector<glm::vec2> texcoords(numVertices, glm::vec2(0.0f));
		std::vector<unsigned int> indices = shape.mesh.indices;

		for (std::size_t i = 0; i < numVertices; ++i)
		{
			positions[i] = glm::vec3(
				shape.mesh.positions[3 * i],
				shape.mesh.positions[3 * i + 1],
				shape.mesh.positions[3 * i + 2]
			);

			if (!shape.mesh.normals.empty())
			{
				normals[i] = glm::vec3(
					shape.mesh.normals[3 * i],
					shape.mesh.normals[3 * i + 1],
					shape.mesh.normals[3 * i + 2]
				);
			}

			if (!shape.mesh.texcoords.empty() && i < shape.mesh.texcoords.size() / 2)
			{
				texcoords[i] = glm::vec2(
					shape.mesh.texcoords[2 * i],
					shape.mesh.texcoords[2 * i + 1]
				);
			}
		}

		setupMesh(positions, normals, texcoords, indices);
		computeBBox(positions);
	}

	Mesh::Mesh(const std::vector<glm::vec3>& positions,
			   const std::vector<glm::vec3>& normals,
			   const std::vector<glm::vec2>& texcoords,
			   const std::vector<unsigned int>& indices)
	{
		setupMesh(positions, normals, texcoords, indices);
		computeBBox(positions);
	}

	void Mesh::setupMesh(const std::vector<glm::vec3>& positions,
						 const std::vector<glm::vec3>& normals,
						 const std::vector<glm::vec2>& texcoords,
						 const std::vector<unsigned int>& indices)
	{
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);

		glGenBuffers(1, &_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

		std::size_t numVertices = positions.size();
		_vertBuf.resize(numVertices * VERTEX_SIZE);

		for (std::size_t i = 0; i < numVertices; ++i)
		{
			std::size_t vi = i * VERTEX_SIZE;

			_vertBuf[vi++] = positions[i].x;
			_vertBuf[vi++] = positions[i].y;
			_vertBuf[vi++] = positions[i].z;

			_vertBuf[vi++] = normals[i].x;
			_vertBuf[vi++] = normals[i].y;
			_vertBuf[vi++] = normals[i].z;

			_vertBuf[vi++] = texcoords[i].x;
			_vertBuf[vi++] = texcoords[i].y;
		}

		glBufferData(GL_ARRAY_BUFFER, _vertBuf.size() * sizeof(float), _vertBuf.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		std::size_t stride = VERTEX_SIZE * sizeof(float);
		std::size_t offset = 0;

		glEnableVertexAttribArray(static_cast<GLuint>(Attrib::Position));
		glVertexAttribPointer(static_cast<GLuint>(Attrib::Position), 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
		offset += 3 * sizeof(float);

		glEnableVertexAttribArray(static_cast<GLuint>(Attrib::Normal));
		glVertexAttribPointer(static_cast<GLuint>(Attrib::Normal), 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
		offset += 3 * sizeof(float);

		glEnableVertexAttribArray(static_cast<GLuint>(Attrib::TexCoord));
		glVertexAttribPointer(static_cast<GLuint>(Attrib::TexCoord), 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);

		glBindVertexArray(0);

		_numIndices = indices.size();
	}

	void Mesh::computeBBox(const std::vector<glm::vec3>& positions)
	{
		_bbox.min = glm::vec3(FLT_MAX);
		_bbox.max = glm::vec3(-FLT_MAX);

		for (const glm::vec3& p : positions)
		{
			_bbox.min.x = std::min(_bbox.min.x, p.x);
			_bbox.min.y = std::min(_bbox.min.y, p.y);
			_bbox.min.z = std::min(_bbox.min.z, p.z);

			_bbox.max.x = std::max(_bbox.max.x, p.x);
			_bbox.max.y = std::max(_bbox.max.y, p.y);
			_bbox.max.z = std::max(_bbox.max.z, p.z);
		}
	}

	void Mesh::draw() const
	{
		glBindVertexArray(_vao);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_numIndices), GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
	}
}