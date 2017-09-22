// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the IMesh, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

#include <hydra/engine.hpp>

#include <iostream>
#include <fstream>

using namespace Hydra;
using namespace Hydra::Renderer;

struct meshInfo {

	std::string name;
	std::vector<glm::vec3> verts;
	std::vector<int> indices;
	std::vector<glm::vec3> norms;
	std::vector<glm::vec3> tangent;
	std::vector<glm::vec3> biNormals;
	std::vector<glm::vec2> uvs;

	std::string fileName;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	GLuint texture;
	GLuint normalMap;
	GLuint movingTexture;
	GLuint VAO;

	glm::mat4 modelMatrix;

};

class GLMeshImpl final : public IMesh {
public:
	GLMeshImpl(std::vector<Vertex> vertices, std::vector<GLuint> indices) {
		_makeBuffers();
		_uploadData(vertices, indices, 0, 0);
	}

	GLMeshImpl(const std::string& file, GLuint modelMatrixBuffer) {
		_loadATTICModel(file.c_str(), modelMatrixBuffer);
	}

	GLMeshImpl(std::vector<Vertex> vertices, std::vector<GLuint> indices, GLuint modelMatrixBuffer, GLuint particleExtraBuffer) {
		_makeBuffers();
		_uploadData(vertices, indices, modelMatrixBuffer, particleExtraBuffer);
	}

	~GLMeshImpl() final {
		GLuint buffers[2] = {_vbo, _ibo};
		glDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers);

		glDeleteVertexArrays(1, &_vao);
	}

	Material& getMaterial() final { return _material; }

	GLuint getID() const final { return _vao; }
	size_t getIndicesCount() const final { return _indicesCount; }
private:
	Material _material;
	GLuint _vao; // Vertex Array
	GLuint _vbo; // Vertices
	GLuint _ibo; // Indices

	size_t _indicesCount;

	void _makeBuffers() {
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		GLuint buffers[2];
		glGenBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
		_vbo = buffers[0];
		_ibo = buffers[1];
	}

	void _uploadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, GLuint modelMatrixBuffer, GLuint particleExtraBuffer) {
		_indicesCount = indices.size();
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(VertexLocation::position);
		glEnableVertexAttribArray(VertexLocation::normal);
		glEnableVertexAttribArray(VertexLocation::color);
		glEnableVertexAttribArray(VertexLocation::uv);
		glEnableVertexAttribArray(VertexLocation::tangent);

		glVertexAttribPointer(VertexLocation::position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glVertexAttribPointer(VertexLocation::normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
		glVertexAttribPointer(VertexLocation::color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
		glVertexAttribPointer(VertexLocation::uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, uv));
		glVertexAttribPointer(VertexLocation::tangent, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));

		if (modelMatrixBuffer) {
			glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
			for (int i = 0; i < 4; i++) {
				glEnableVertexAttribArray(VertexLocation::modelMatrix + i);
				glVertexAttribPointer(VertexLocation::modelMatrix + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4) * i));
				glVertexAttribDivisor(VertexLocation::modelMatrix + i, 1);
			}
		}
		if (particleExtraBuffer) {
			glBindBuffer(GL_ARRAY_BUFFER, particleExtraBuffer);
			for (int i = 0; i < 3; i++) {
				glEnableVertexAttribArray(VertexLocation::textureOffset1 + i);
				glVertexAttribPointer(VertexLocation::textureOffset1 + i, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)(sizeof(glm::vec2) * i));
				glVertexAttribDivisor(VertexLocation::textureOffset1 + i, 1);
			}
		}
	}

	void _loadATTICModel(const char* filePath, GLuint modelMatrixBuffer) {
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		glm::vec3 vec3;
		glm::vec2 vec2;
		//Open the file
		std::ifstream in(filePath, std::ios::binary);
		if (!in.good()) {
			IEngine::getInstance()->log(LogLevel::error, "Could not load model %s", filePath);
			return;
		}

		int nrOfMeshes = 0;
		//Read the number of meshes in the file
		in.read(reinterpret_cast<char*>(&nrOfMeshes), sizeof(int));

		for (int i = 0; i < nrOfMeshes; i++) {
			//meshInfo *info = new meshInfo;

			//Get the name by first getting the number of chars, then read
			//that ammount of chars
			std::string name = "";
			int nrOfChars = 0;
			in.read(reinterpret_cast<char*>(&nrOfChars), sizeof(int));
			char *tempName;
			tempName = new char[nrOfChars];
			in.read(tempName, nrOfChars);
			name.append(tempName, nrOfChars);

			std::cout << name << std::endl;
			delete[] tempName;

			int nrOfControlpoints = 0;
			//Read the number of vertices on the mesh
			in.read(reinterpret_cast<char*>(&nrOfControlpoints), sizeof(int));
			//info->name = name;

			for (int k = 0; k < nrOfControlpoints; k++) {
				Vertex newVertex = {};
				vec3 = glm::vec3(0);
				//Read the Vertex for the vertex
				in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
				newVertex.position = vec3;
				//Read the Normal for the vertex
				in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
				newVertex.normal = vec3;
				//Read the Tangents for the vertex
				in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
				newVertex.tangent= vec3;
				//Read the UV for the vertex
				in.read(reinterpret_cast<char*>(&vec2), sizeof(vec2));
				newVertex.uv = vec2;

				//Read the Vertecies for the primitive
				//for (int h = 0; h < 3; h++) {
				//	in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
				//	info->verts.push_back(vec3);
				//}
				//
				////Read the Normals for the primitive
				//for (int h = 0; h < 3; h++) {
				//	in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
				//	info->norms.push_back(vec3);
				//
				//	//Read the Tangents for the primitive
				//	in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
				//	info->tangent.push_back(vec3);
				//
				//	//Read the BiNormals for the primitive
				//	in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
				//	info->biNormals.push_back(vec3);
				//}
				////Read the UVs for the primitive
				//for (int h = 0; h < 3; h++) {
				//	in.read(reinterpret_cast<char*>(&vec2), sizeof(vec2));
				//	info->uvs.push_back(vec2);
				//}
				//
				//
				//newVertex->position = info->verts[k];
				//newVertex->normal = info->norms[info->norms.size() - (3 - q)];
				//newVertex->tangent = info->tangent[info->tangent.size() - (3 - q)];
				//newVertex->uv = info->uvs[info->uvs.size() - (3 - q)];

				vertices.push_back(newVertex);
			}

			int nrOfPrimitives = 0;
			//Read the number of triangles on the mesh for the indices
			in.read(reinterpret_cast<char*>(&nrOfPrimitives), sizeof(int));
			for (int w = 0; w < nrOfPrimitives; w++) {
				//Read the indices
				for (int q = 0; q < 3; q++) {
					int indexData = 0;
					in.read(reinterpret_cast<char*>(&indexData), sizeof(int));
					//info->indices.push_back(indexData);
					indices.push_back(indexData);
				}
			}

			//Read the materials for the mesh as well as the Texture file name
			int fileNameLength = 0;
			glm::vec3 diffuse;
			float specular = 0;

			std::string fileName = "";
			in.read(reinterpret_cast<char*>(&fileNameLength), sizeof(int));
			char *tempFileName;
			tempFileName = new char[fileNameLength];
			in.read(tempFileName, fileNameLength);
			fileName.append(tempFileName, fileNameLength);

			delete[] tempFileName;

			//Read the diffuse and specular value
			in.read(reinterpret_cast<char*>(&diffuse), sizeof(diffuse));
			in.read(reinterpret_cast<char*>(&specular), sizeof(specular));

			//info->ambient = glm::vec3(0.15);
			//info->diffuse = diffuse;
			//info->specular = glm::vec3(specular);

			for (size_t d = 0; d < vertices.size(); d++)
				vertices[d].color = diffuse;

			vec3 = glm::vec3(0);
			//Read the position, rotation and scale values
			in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
			//info->position = vec3;
			in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
			//info->rotation = vec3;
			in.read(reinterpret_cast<char*>(&vec3), sizeof(vec3));
			//info->scale = vec3;

			bool hasAnimation = false;
			//Read if the mesh has animation, in this case we should
			//do a different type of rendering in the vertex shader
			in.read(reinterpret_cast<char*>(&hasAnimation), sizeof(bool));

			//const char *test = fileName.c_str();
			//info->texture = createTexture(test);
			//glGenVertexArrays(1, &info->VAO);
			//ATTICMeshes.push_back(info);

			_makeBuffers();
			_uploadData(vertices, indices, modelMatrixBuffer, 0);

			//delete info;
		}
	}
};

std::unique_ptr<IMesh> GLMesh::create(const std::string& file, IRenderer* renderer) {
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(file, *static_cast<GLuint*>(renderer->getModelMatrixBuffer())));
}

std::unique_ptr<IMesh> GLMesh::createQuad(IRenderer* renderer) {
	std::vector<Vertex> vertices{
		Vertex{ { -0.5, 0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { 0.5, 0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { 0.5, -0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { -0.5, -0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } }
	};
	std::vector<GLuint> indices{ 0, 2, 1, 2, 0, 3 };
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(vertices, indices, *static_cast<GLuint*>(renderer->getModelMatrixBuffer()), *static_cast<GLuint*>(renderer->getParticleExtraBuffer())));
}

std::unique_ptr<IMesh> GLMesh::createFullscreenQuad() {
	std::vector<Vertex> vertices{
		Vertex{{-1, 1, 0}, {0, 0, -1}, {1, 1, 1}, {0, 1}, {0, 0, 0}},
		Vertex{{1, 1, 0}, {0, 0, -1}, {1, 1, 1}, {1, 1}, {0, 0, 0}},
		Vertex{{1, -1, 0}, {0, 0, -1}, {1, 1, 1}, {1, 0}, {0, 0, 0}},
		Vertex{{-1, -1, 0}, {0, 0, -1}, {1, 1, 1}, {0, 0}, {0, 0, 0}}
	};
	std::vector<GLuint> indices{0, 2, 1, 2, 0, 3};
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(vertices, indices));
}
