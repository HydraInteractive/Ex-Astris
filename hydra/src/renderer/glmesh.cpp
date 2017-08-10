#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <hydra/engine.hpp>

using namespace Hydra::Renderer;

class GLMeshImpl final : public IMesh {
public:
	GLMeshImpl(const std::string& file, GLuint modelMatrixBuffer) {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Assimp::Importer importer;
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
		auto flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_GenNormals;
		const aiScene* scene = importer.ReadFile(file, flags);

		if (!scene) {
			fprintf(stderr, "Could not load model %s\n", file.c_str());
			throw "Could not load model";
		}

		_loadModel(scene, modelMatrixBuffer);
		_material.diffuse = _getTexture(scene, file);
		_material.normal = Hydra::IEngine::getInstance()->getTextureLoader()->getTexture("assets/textures/errorNormal.png");	
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

	void _uploadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, GLuint modelMatrixBuffer) {
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

		glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
		for (int i = 0; i < 4; i++) {
			glEnableVertexAttribArray(VertexLocation::modelMatrix + i);
			glVertexAttribPointer(VertexLocation::modelMatrix + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid*)(sizeof(glm::vec4) * i));
			glVertexAttribDivisor(VertexLocation::modelMatrix + i, 1);
		}
	}

	bool _hasVertexColors(aiMesh* mesh, uint32_t pIndex) {
		if (pIndex >= AI_MAX_NUMBER_OF_COLOR_SETS)
			return false;
		else
			return mesh->mColors[pIndex] != NULL && mesh->mNumVertices > 0;
	}

	bool _hasTextureCoords(aiMesh* mesh, uint32_t pIndex) {
		if (pIndex >= AI_MAX_NUMBER_OF_TEXTURECOORDS)
			return false;
		else
			return mesh->mTextureCoords[pIndex] != NULL && mesh->mNumVertices > 0;
	}

	void _loadModel(const aiScene* scene, GLuint modelMatrixBuffer) {
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		uint32_t counterVertices = 0;
		for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* assimpMesh = scene->mMeshes[i];
			bool hasColors = _hasVertexColors(assimpMesh, 0);
			bool hasUV = _hasTextureCoords(assimpMesh, 0);
			for (uint32_t j = 0; j < assimpMesh->mNumVertices; j++) {
				Vertex vertex;

				aiVector3D p = assimpMesh->mVertices[j];
				vertex.position = glm::vec3{p.x, p.z, -p.y};

				p = assimpMesh->mNormals[j];
				vertex.normal = glm::vec3{p.x, p.z, -p.y};

				if (hasColors) {
					aiColor4D c = assimpMesh->mColors[0][j];
					vertex.color = glm::vec3{c.r, c.g, c.b};
				} else
					vertex.color = glm::vec3{1.f, 1.f, 1.f};

				if (hasUV) {
					aiVector3D uv = assimpMesh->mTextureCoords[0][j];
					vertex.uv = glm::vec2{uv.x, uv.y};
				} else
					vertex.uv = glm::vec2{j & 2, (j / 2) % 2};

				if (assimpMesh->mTangents) {
					p = assimpMesh->mTangents[j];
					vertex.tangent = glm::vec3{p.x, p.z, -p.y};
				}

				vertices.push_back(vertex);
			}

			for (uint32_t j = 0; j < assimpMesh->mNumFaces; j++) {
				uint32_t* face = assimpMesh->mFaces[j].mIndices;
				indices.push_back(counterVertices + face[0]);
				indices.push_back(counterVertices + face[1]);
				indices.push_back(counterVertices + face[2]);
			}
			counterVertices += assimpMesh->mNumVertices;
		}

		_indicesCount = indices.size();

		_makeBuffers();
		_uploadData(vertices, indices, modelMatrixBuffer);
	}

	std::shared_ptr<ITexture> _getTexture(const aiScene* scene, const std::string& filename) {
		if (scene->mNumMaterials == 0)
			return Hydra::IEngine::getInstance()->getTextureLoader()->getTexture(""); // Error texture
		const aiMaterial* pMaterial = scene->mMaterials[0];
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) == 0)
			return Hydra::IEngine::getInstance()->getTextureLoader()->getTexture(""); // Error texture

		aiString path;

		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS)
			return Hydra::IEngine::getInstance()->getTextureLoader()->getTexture(""); // Error texture

		if (path.data[0] == '*') {
			unsigned int id = atoi(path.data + 1);
			printf("Embedded texture: %u(%s)\n", id, path.data);
			if (scene->mNumTextures < id)
				return Hydra::IEngine::getInstance()->getTextureLoader()->getTexture(""); // Error texture
			aiTexture* tex = scene->mTextures[id];

			printf("Texture: \n");
			printf("\tmWidth: %u\n", tex->mWidth);
			printf("\tmHeight: %u\n", tex->mHeight);
			printf("\tachFormatHint: %s\n", tex->achFormatHint);
			printf("\tpcData: %p\n", (void*)tex->pcData);
			if (tex->mHeight)
				return GLTexture::createFromData(tex->mWidth, tex->mHeight, (void*)tex->pcData);
			else
				return GLTexture::createFromDataExt(tex->achFormatHint, (void*)tex->pcData, tex->mWidth);
		} else {
			std::string fullPath = filename.substr(0, filename.find_last_of("/\\") + 1) + path.data; // TODO: fix path
			printf("External texture: %s\n", fullPath.c_str());
			return Hydra::IEngine::getInstance()->getTextureLoader()->getTexture(fullPath);
		}
	}
};

std::unique_ptr<IMesh> GLMesh::create(const std::string& file, IRenderer* renderer) {
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(file, *static_cast<GLuint*>(renderer->getModelMatrixBuffer())));
}
