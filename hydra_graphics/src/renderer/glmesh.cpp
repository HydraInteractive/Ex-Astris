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
#include <glm/gtx/transform.hpp>
#include <glad/glad.h>

#include <hydra/engine.hpp>
#include <hydra/component/textcomponent.hpp> // Little Fuling LmAO

#include <iostream>
#include <fstream>

using namespace Hydra;
using namespace Hydra::Renderer;

class GLMeshImpl final : public IMesh {
public:
	GLMeshImpl(std::vector<Vertex> vertices, std::vector<GLuint> indices) {
		_file = "(internal data)";
		_makeBuffers();
		_uploadData(vertices, indices, false, 0, 0, 0);
	}

	GLMeshImpl(const std::string& file, GLuint modelMatrixBuffer) {
		_file = file;
		_currentFrame = 1;
		_currentAnimationIndex = 0;
		_animationCounter = 0;
		_animDataUploaded = false;
		_loadATTICModel(file.c_str(), modelMatrixBuffer);
	}

	GLMeshImpl(std::vector<Vertex> vertices, std::vector<GLuint> indices, bool animation, GLuint modelMatrixBuffer, GLuint particleExtraBuffer, GLuint textExtraBuffer) {
		_file = "(internal data, animation, modelMatrixBuffer, ParticleExtraBuffer)";
		_makeBuffers();
		_uploadData(vertices, indices, animation, modelMatrixBuffer, particleExtraBuffer, textExtraBuffer);
	}

	~GLMeshImpl() final {
		GLuint buffers[2] = {_vbo, _ibo};
		glDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers);

		glDeleteVertexArrays(1, &_vao);
		
		for (size_t i = 0; i < 7; i++) {
			for (size_t k = 0; k < skeleton[i].size(); k++) {
				delete skeleton[i][k];
			}
		}
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
	glm::mat4 getTransformationMatrices(int currAnimIdx, int joint, int currentFrame) final { return _finishedMatrices[currAnimIdx][joint]->finishedTransformMat[currentFrame]; }
	int getNrOfJoints(int currAnimIdx) final { return _finishedMatrices[currAnimIdx][0]->nrOfClusters; }
	int getCurrentKeyframe() final { return _currentFrame; }
	int getMaxFramesForAnimation(int currAnimIdx) final { return _finishedMatrices[currAnimIdx][0]->nrOfKeys; }
	int getCurrentAnimationIndex() final { return _currentAnimationIndex; }
	void setCurrentKeyframe(int frame) { _currentFrame = frame; }
	void setAnimationIndex(int index) { _currentAnimationIndex = index; }

	GLuint getID() const final { return _vao; }
	size_t getIndicesCount() const final { return _indicesCount; }
	float& getAnimationCounter() { return _animationCounter; }


private:
	std::string _file = "(null)";
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
	float _animationCounter;
	bool _animDataUploaded;

	void _makeBuffers() {
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		GLuint buffers[2];
		glGenBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
		_vbo = buffers[0];
		_ibo = buffers[1];
	}

	void _uploadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, bool animation, GLuint modelMatrixBuffer, GLuint particleExtraBuffer, GLuint textExtraBuffer) {
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
		if (textExtraBuffer) {
			glBindBuffer(GL_ARRAY_BUFFER, textExtraBuffer);
			glEnableVertexAttribArray(VertexLocation::charRect);
			glVertexAttribPointer(VertexLocation::charRect, 4, GL_FLOAT, GL_FALSE, sizeof(Hydra::Renderer::CharRenderInfo), (GLvoid*)offsetof(Hydra::Renderer::CharRenderInfo, charRect));
			glVertexAttribDivisor(VertexLocation::charRect, 1);

			glEnableVertexAttribArray(VertexLocation::charPos);
			glVertexAttribPointer(VertexLocation::charPos, 3, GL_FLOAT, GL_FALSE, sizeof(Hydra::Renderer::CharRenderInfo), (GLvoid*)offsetof(Hydra::Renderer::CharRenderInfo, charPos));
			glVertexAttribDivisor(VertexLocation::charPos, 1);
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
			std::string glowName = "";
			in.read(reinterpret_cast<char*>(&fileNameLength), sizeof(int));
			char *tempFileName;
			tempFileName = new char[fileNameLength];
			in.read(tempFileName, fileNameLength);

			//If I acidentally used a psd file, just take a png file instead
			fileName.append(tempFileName, fileNameLength);
			char lastChar = fileName.back();
			if (lastChar == 'd') {
				fileName.erase(fileName.size() - 2, fileName.size());
				fileName.append("ng", fileNameLength - 2);
			}
			if (name != "AlienBossModel") {
				if (fileName != "NULL" && fileNameLength != 0)
					_material.diffuse = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/" + fileName);
				else
					_material.diffuse = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/Floor_specular.png");
			}
			else
				_material.diffuse = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/AlienBossTexture.png");
				
			// _material.diffuse = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textuers/1x1gray.png");

			delete[] tempFileName;

			_material.specular = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/1x1Gray.png");

			//Read the diffuse and specular value
			in.read(reinterpret_cast<char*>(&diffuse), sizeof(diffuse));
			in.read(reinterpret_cast<char*>(&specular), sizeof(specular));

			//Read the normal Texture
			fileName = "";
			in.read(reinterpret_cast<char*>(&fileNameLength), sizeof(int));
			char *tempNormalFileName;
			tempNormalFileName = new char[fileNameLength];
			in.read(tempNormalFileName, fileNameLength);
			fileName.append(tempNormalFileName, fileNameLength);
			if (fileName != "NULL" && fileNameLength != 0)
				_material.normal = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/normals/" + fileName);
			else
				_material.normal = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/normals/1x1ErrorNormal.png");

			delete[] tempNormalFileName;

			//Read the glow Texture
			fileName = "";
			in.read(reinterpret_cast<char*>(&fileNameLength), sizeof(int));
			char *tempGlowFileName;
			tempGlowFileName = new char[fileNameLength];
			in.read(tempGlowFileName, fileNameLength);
			fileName.append(tempGlowFileName, fileNameLength);
			if (fileName != "NULL" && fileNameLength != 0)
				_material.glow = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/glow/" + fileName);
			else
				_material.glow = IEngine::getInstance()->getState()->getTextureLoader()->getTexture("assets/textures/glow/errorGlow.png");
			delete[] tempGlowFileName;

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
				_uploadData(vertices, indices, true, modelMatrixBuffer, 0, 0);

			}
			else {
				_meshHasAnimation = false;
				_uploadData(vertices, indices, false, modelMatrixBuffer, 0, 0);
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

			delete[] tempName;

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

std::unique_ptr<IMesh> GLMesh::createParticleQuad(IRenderer* renderer) {
	std::vector<Vertex> vertices{
		Vertex{ { -0.5, 0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { 0.5, 0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { 0.5, -0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { -0.5, -0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } }
	};
	std::vector<GLuint> indices{ 0, 2, 1, 2, 0, 3 };
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(vertices, indices, false, *static_cast<GLuint*>(renderer->getModelMatrixBuffer()), *static_cast<GLuint*>(renderer->getParticleExtraBuffer()), 0));
}

std::unique_ptr<IMesh> GLMesh::createTextQuad(IRenderer* renderer) {
	std::vector<Vertex> vertices{
		Vertex{ { -0.5, 0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { 0.5, 0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { 0.5, -0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { -0.5, -0.5, 0 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } }
	};
	printf("BIG FAT HARDCORE, I WANT IT INSIDE OF ME\n");
	std::vector<GLuint> indices{ 0, 2, 1, 2, 0, 3 };
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(vertices, indices, false, 0, 0, *static_cast<GLuint*>(renderer->getTextExtraBuffer())));
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

std::unique_ptr<IMesh> GLMesh::createCube() {
	std::vector<Vertex> vertices{
		Vertex{ { -1, -1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, -1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, 1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { -1, 1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } },

		Vertex{ { 1, 1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, 1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, -1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { 1, -1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } },

		Vertex{ { -1, -1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, -1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, 1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { -1, 1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } },

		Vertex{ { -1, -1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { -1, -1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { -1, 1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { -1, 1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } },

		Vertex{ { 1, 1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { -1, 1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { -1, 1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { 1, 1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } },

		Vertex{ { -1, -1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, -1, -1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 1 },{ 0, 0, 0 } },
		Vertex{ { 1, -1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 1, 0 },{ 0, 0, 0 } },
		Vertex{ { -1, -1, 1 },{ 0, 0, -1 },{ 1, 1, 1 },{ 0, 0 },{ 0, 0, 0 } }
	};

	std::vector<GLuint> indices{ 0, 1, 2, 0, 2, 3, //front
		4, 5, 6, 4, 6, 7, // right
		8, 9, 10, 8, 10, 11, // back
		12, 13, 14, 12, 14, 15, // left
		16, 17, 18, 16, 18, 19, // upper
		20, 21, 22, 20, 22, 23 }; // bottom

	return std::unique_ptr<IMesh>(new ::GLMeshImpl(vertices, indices));
}
