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

struct skelInfo {

	int nrOfClusters;
	std::string jointName;
	glm::vec4 globalBindPose0;
	glm::vec4 globalBindPose1;
	glm::vec4 globalBindPose2;
	glm::vec4 globalBindPose3;

	glm::mat4 globalBindPosMat;

	std::vector<glm::vec4> transRow0;
	std::vector<glm::vec4> transRow1;
	std::vector<glm::vec4> transRow2;
	std::vector<glm::vec4> transRow3;

	std::vector<glm::vec4> finishedTransRow0;
	std::vector<glm::vec4> finishedTransRow1;
	std::vector<glm::vec4> finishedTransRow2;
	std::vector<glm::vec4> finishedTransRow3;

	std::vector<glm::mat4> transformMat;
	std::vector<glm::mat4> finishedTransformMat;

	int nrOfKeys;
};
std::vector<skelInfo*> skeleton[7];
int animationIndex;

struct weights {
	int nrOfIndices;
	std::vector<int> indexPos;
	std::vector<glm::vec3> polygonVerteciesIndex;
	std::vector<glm::ivec4> controllers;
	std::vector<glm::vec4> weightsInfluence;
};
weights weightInfo;

class GLMeshImpl final : public IMesh {
public:
	GLMeshImpl(std::vector<Vertex> vertices, std::vector<GLuint> indices) {
		_makeBuffers();
		_uploadData(vertices, indices, false, 0, 0);
	}

	GLMeshImpl(const std::string& file, GLuint modelMatrixBuffer) {

		_currentFrame = 1;
		_currentAnimationIndex = 0;
		_loadATTICModel(file.c_str(), modelMatrixBuffer);
	}

	GLMeshImpl(std::vector<Vertex> vertices, std::vector<GLuint> indices, bool animation, GLuint modelMatrixBuffer, GLuint particleExtraBuffer) {
		_makeBuffers();
		_uploadData(vertices, indices, animation, modelMatrixBuffer, particleExtraBuffer);
	}


	~GLMeshImpl() final {
		GLuint buffers[2] = {_vbo, _ibo};
		glDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers);

		glDeleteVertexArrays(1, &_vao);
	}

	struct skelInfo {

		int nrOfClusters;
		std::string jointName;
		glm::vec4 globalBindPose0;
		glm::vec4 globalBindPose1;
		glm::vec4 globalBindPose2;
		glm::vec4 globalBindPose3;

		glm::mat4 globalBindPosMat;

		std::vector<glm::vec4> transRow0;
		std::vector<glm::vec4> transRow1;
		std::vector<glm::vec4> transRow2;
		std::vector<glm::vec4> transRow3;

		std::vector<glm::vec4> finishedTransRow0;
		std::vector<glm::vec4> finishedTransRow1;
		std::vector<glm::vec4> finishedTransRow2;
		std::vector<glm::vec4> finishedTransRow3;

		std::vector<glm::mat4> transformMat;
		std::vector<glm::mat4> finishedTransformMat;

		int nrOfKeys;
	};
	std::vector<skelInfo*> skeleton[7];
	int animationIndex;

	struct weights {
		int nrOfIndices;
		std::vector<int> indexPos;
		std::vector<glm::vec3> polygonVerteciesIndex;
		std::vector<glm::ivec4> controllers;
		std::vector<glm::vec4> weightsInfluence;
	};

	weights weightInfo;

	Material& getMaterial() final { return _material; }

	bool hasAnimation() final { return _meshHasAnimation; }
	glm::mat4 getTransformationMatrices(int joint) final { return _finishedMatrices[_currentAnimationIndex][joint]->finishedTransformMat[_currentFrame - 1]; }
	int getNrOfJoints() final { return _finishedMatrices[_currentAnimationIndex][0]->nrOfClusters; }
	int getCurrentKeyframe() final { return _currentFrame; }
	int getMaxFramesForAnimation() final { return _finishedMatrices[_currentAnimationIndex][0]->nrOfKeys; }
	int getCurrentAnimationIndex() final { return _currentAnimationIndex; }
	void setCurrentKeyframe(int frame) { _currentFrame = frame; }
	void setAnimationIndex(int index) { _currentAnimationIndex = index; }

	GLuint getID() const final { return _vao; }
	size_t getIndicesCount() const final { return _indicesCount; }

private:
	Material _material;
	GLuint _vao; // Vertex Array
	GLuint _vbo; // Vertices
	GLuint _ibo; // Indices
	size_t _indicesCount;
	
	std::vector<skelInfo*> _finishedMatrices[7];
	bool _meshHasAnimation = false;

	int _nrOfJoints;
	int _maxFramesForAnimation;
	int _currentFrame;
	int _currentAnimationIndex;

	void _makeBuffers() {
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		GLuint buffers[2];
		glGenBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
		_vbo = buffers[0];
		_ibo = buffers[1];
	}

	void _uploadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, bool animation, GLuint modelMatrixBuffer, GLuint particleExtraBuffer) {
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
		if (animation) {
			glEnableVertexAttribArray(VertexLocation::influences);
			glEnableVertexAttribArray(VertexLocation::controllers);
		}

		glVertexAttribPointer(VertexLocation::position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glVertexAttribPointer(VertexLocation::normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
		glVertexAttribPointer(VertexLocation::color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
		glVertexAttribPointer(VertexLocation::uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, uv));
		glVertexAttribPointer(VertexLocation::tangent, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));
		if (animation) {
			glVertexAttribPointer(VertexLocation::influences, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, influences));
			glVertexAttribPointer(VertexLocation::controllers, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, controllers));
		}

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
				glVertexAttribPointer(VertexLocation::textureOffset1 + i, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 3, (GLvoid*)(sizeof(glm::vec2) * i));
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

			_indicesCount = indices.size();
			_makeBuffers();

			if (hasAnimation) {

				_meshHasAnimation = true;
				int nrOfAnimationFiles;
				in.read(reinterpret_cast<char*>(&nrOfAnimationFiles), sizeof(int));
				bool test = true;
				std::string animationFilePath = "assets/objects/characters/";
				std::string animationFileName;
				int nrOfFileChars = 0;

				//Read the weight info
				in.read(reinterpret_cast<char*>(&nrOfFileChars), sizeof(int));
				char *tempAnimationFileName;
				tempAnimationFileName = new char[nrOfFileChars];
				in.read(tempAnimationFileName, nrOfFileChars);
				animationFileName.append(tempAnimationFileName, nrOfFileChars);
				delete[] tempAnimationFileName;
				animationFileName += ".wATTIC";
				animationFilePath += animationFileName;

				_loadWeight(animationFilePath.c_str(), vertices);

				//Read all the skeleton info. In other words, all different animations
				for (int animationFile = 0; animationFile < nrOfAnimationFiles; animationFile++) {

					in.read(reinterpret_cast<char*>(&nrOfFileChars), sizeof(int));
					animationFilePath = "assets/objects/characters/";
					animationFileName = "";
					tempAnimationFileName = new char[nrOfFileChars];
					in.read(tempAnimationFileName, nrOfFileChars);
					animationFileName.append(tempAnimationFileName, nrOfFileChars);
					animationFileName += ".sATTIC";
					animationFilePath += animationFileName;

					delete[] tempAnimationFileName;

					_loadSkeleton(animationFilePath.c_str(), vertices);


				}
				_uploadData(vertices, indices, true, modelMatrixBuffer, 0);

			}
			else {
				_meshHasAnimation = false;
				_uploadData(vertices, indices, false, modelMatrixBuffer, 0);
			}

		}
	
	}

	void _loadWeight(const char* filePath, std::vector<Vertex>& vertices) {
		std::ifstream in(filePath, std::ios::binary);

		int nrOfCtrlPoints = 0;
		in.read(reinterpret_cast<char*>(&nrOfCtrlPoints), sizeof(int));

		//int polygonIndex[3];

		glm::ivec3 polygonVertexIndex;
		glm::ivec4 controllers;
		glm::vec4 weightInfluences;

		for (int k = 0; k < nrOfCtrlPoints; k++) {
			for (int i = 0; i < 4; i++) {
				in.read(reinterpret_cast<char*>(&controllers[i]), sizeof(int));

				in.read(reinterpret_cast<char*>(&weightInfluences[i]), sizeof(float));
			}
			vertices[k].controllers = controllers;
			vertices[k].influences = weightInfluences;
		}
		
	}

	void _loadSkeleton(const char* filePath, const std::vector<Vertex>& vertices) {
		//First nrOfClusters
		std::ifstream in(filePath, std::ios::binary);
		int clusterNr = 0;
		int indexNmr = 0;
		int nrOfKeyframes = 0;

		in.read(reinterpret_cast<char*>(&indexNmr), sizeof(int));
		in.read(reinterpret_cast<char*>(&nrOfKeyframes), sizeof(int));
		in.read(reinterpret_cast<char*>(&clusterNr), sizeof(int));

		for (int i = 0; i < clusterNr; i++) {

			//Get the name
			std::string name = "";
			int nrOfChars = 0;
			in.read(reinterpret_cast<char*>(&nrOfChars), sizeof(int));
			char *tempName;
			tempName = new char[nrOfChars];
			in.read(tempName, nrOfChars);
			name.append(tempName, nrOfChars);

			delete tempName;

			skelInfo *info = new skelInfo;
			info->nrOfKeys = nrOfKeyframes;
			info->nrOfClusters = clusterNr;
			info->jointName = name;

			glm::vec4 globalBindVec0;
			glm::vec4 globalBindVec1;
			glm::vec4 globalBindVec2;
			glm::vec4 globalBindVec3;

			in.read(reinterpret_cast<char*>(&globalBindVec0), sizeof(globalBindVec0));
			info->globalBindPose0 = globalBindVec0;

			in.read(reinterpret_cast<char*>(&globalBindVec1), sizeof(globalBindVec1));
			info->globalBindPose1 = globalBindVec1;

			in.read(reinterpret_cast<char*>(&globalBindVec2), sizeof(globalBindVec2));
			info->globalBindPose2 = globalBindVec2;

			in.read(reinterpret_cast<char*>(&globalBindVec3), sizeof(globalBindVec3));
			info->globalBindPose3 = globalBindVec3;

			glm::mat4 tempBindMat;
			tempBindMat = glm::mat4(info->globalBindPose0, info->globalBindPose1,
				info->globalBindPose2, info->globalBindPose3);
			info->globalBindPosMat = tempBindMat;

			for (int o = 0; o < nrOfKeyframes; o++) {

				glm::vec4 glmVec0;
				glm::vec4 glmVec1;
				glm::vec4 glmVec2;
				glm::vec4 glmVec3;
				in.read(reinterpret_cast<char*>(&glmVec0), sizeof(glmVec0));
				info->transRow0.push_back(glmVec0);

				in.read(reinterpret_cast<char*>(&glmVec1), sizeof(glmVec1));
				info->transRow1.push_back(glmVec1);

				in.read(reinterpret_cast<char*>(&glmVec2), sizeof(glmVec2));
				info->transRow2.push_back(glmVec2);

				in.read(reinterpret_cast<char*>(&glmVec3), sizeof(glmVec3));
				info->transRow3.push_back(glmVec3);

				glm::mat4 tempMap = glm::mat4(glmVec0, glmVec1, glmVec2, glmVec3);
				info->transformMat.push_back(tempMap);

				in.read(reinterpret_cast<char*>(&glmVec0), sizeof(glmVec0));
				info->finishedTransRow0.push_back(glmVec0);

				in.read(reinterpret_cast<char*>(&glmVec1), sizeof(glmVec1));
				info->finishedTransRow1.push_back(glmVec1);

				in.read(reinterpret_cast<char*>(&glmVec2), sizeof(glmVec2));
				info->finishedTransRow2.push_back(glmVec2);

				in.read(reinterpret_cast<char*>(&glmVec3), sizeof(glmVec3));
				info->finishedTransRow3.push_back(glmVec3);

				tempMap = glm::mat4(glmVec0, glmVec1, glmVec2, glmVec3);
				info->finishedTransformMat.push_back(tempMap);

			}
			_finishedMatrices[indexNmr].push_back(info);
			skeleton[indexNmr].push_back(info);
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
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(vertices, indices, false, *static_cast<GLuint*>(renderer->getModelMatrixBuffer()), *static_cast<GLuint*>(renderer->getParticleExtraBuffer())));
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