#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

// NOTE: This GLRenderer depends on SDL aswell!
//       Maybe abstract way this in the future.
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <hydra/engine.hpp>

using namespace Hydra::Renderer;

extern Hydra::IEngine* engineInstance;

static void glDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

class GLMeshImpl : public IMesh {
public:
	GLMeshImpl(const std::string& file) {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Assimp::Importer importer;
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
		auto flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_GenNormals | aiProcess_FlipUVs;
		const aiScene* scene = importer.ReadFile(file, flags);

		if (!scene) {
			fprintf(stderr, "Could not load model %s\n", file.c_str());
			throw "Could not load model";
		}

		_loadModel(scene);
		_material.diffuse = _getTexture(scene, file);
		_material.normal = engineInstance->getTextureLoader()->getTexture("assets/textures/errorNormal.png");	
	}

	~GLMeshImpl() final {
		GLuint buffers[2] = {_vbo, _ibo};
		glDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers);

		glDeleteVertexArrays(1, &_vao);
	}

	Material& getMaterial() final { return _material; }

	GLuint getVAO() { return _vao; }
	size_t getIndicesCount() { return _indicesCount; }

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

	void _uploadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
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

	void _loadModel(const aiScene* scene) {
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
		_uploadData(vertices, indices);
	}

	std::shared_ptr<ITexture> _getTexture(const aiScene* scene, const std::string& filename) {
		if (scene->mNumMaterials == 0)
			return engineInstance->getTextureLoader()->getTexture(""); // Error texture
		const aiMaterial* pMaterial = scene->mMaterials[0];
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) == 0)
			return engineInstance->getTextureLoader()->getTexture(""); // Error texture

		aiString path;

		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS)
			return engineInstance->getTextureLoader()->getTexture(""); // Error texture

		if (path.data[0] == '*') {
			unsigned int id = atoi(path.data + 1);
			printf("Embedded texture: %u(%s)\n", id, path.data);
			if (scene->mNumTextures < id)
				return engineInstance->getTextureLoader()->getTexture(""); // Error texture
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
			return engineInstance->getTextureLoader()->getTexture(fullPath);
		}
	}
};

class GLTextureImpl : public ITexture {
public:
	GLTextureImpl(GLuint texture) : _own(false), _texture(texture) { }

	GLTextureImpl(const std::string& file) : _own(true) {
		SDL_Surface* surface = IMG_Load(file.c_str());
		if (!surface)
			throw "Texture failed to load!";

		GLenum format;

		int nOfColors = surface->format->BytesPerPixel;
		if (nOfColors == 4) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGBA;
			else
				format = GL_BGRA;
		} else if (nOfColors == 3) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGB;
			else
				format = GL_BGR;
		} else {
			SDL_Surface* newSurf = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0);
			if (!newSurf)
				throw "Unknown texture format";
			SDL_FreeSurface(surface);
			surface = newSurf;
			format = GL_RGB;
		}

		_setData(format, surface->w, surface->h, surface->pixels);

		SDL_FreeSurface(surface);
	}

	GLTextureImpl(uint32_t width, uint32_t height, void* data) : _own(true) {
		_setData(GL_RGBA, width, height, data);
	}

	GLTextureImpl(const char* ext, void* data, uint32_t size) : _own(true) {
		SDL_Surface* surface = IMG_LoadTyped_RW(SDL_RWFromConstMem(data, size), 1, ext);
		if (!surface)
			throw "Texture failed to load!";

		GLenum format;

		int nOfColors = surface->format->BytesPerPixel;
		if (nOfColors == 4) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGBA;
			else
				format = GL_BGRA;
		} else if (nOfColors == 3) {
			if (surface->format->Rmask == 0x000000ff)
				format = GL_RGB;
			else
				format = GL_BGR;
		} else {
			SDL_Surface* newSurf = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0);
			if (!newSurf)
				throw "Unknown texture format";
			SDL_FreeSurface(surface);
			surface = newSurf;
			format = GL_RGB;
		}

		_setData(format, surface->w, surface->h, surface->pixels);

		SDL_FreeSurface(surface);
	}

	~GLTextureImpl() final {
		if (_own)
			glDeleteTextures(1, &_texture);
	}

	uint32_t getID() { return _texture; }
private:
	bool _own;
	GLuint _texture;

	void _setData(GLenum format, GLuint w, GLuint h, const void* pixels) {
		glGenTextures(1, &_texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);

		// TODO: be able to change this
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
	}
};

class GLRendererImpl final : public IRenderer {
public:
	GLRendererImpl(Hydra::View::IView& view) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		_glContext = SDL_GL_CreateContext(_window = static_cast<SDL_Window*>(view.getHandler()));
		_loadGLAD();

		glDebugMessageCallback(&glDebugLog, nullptr);
		glEnable(GL_DEBUG_OUTPUT);

		SDL_GL_SetSwapInterval(0);
	}

	~GLRendererImpl() final {
		SDL_GL_DeleteContext(_glContext);
	}

	void setRenderOrder(RenderOrder renderOrder) final { _renderOrder = renderOrder; }

	void bind(IRenderTarget& renderTarget) final {
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.getID());
	}

	void clear(glm::vec4 clearColor) final {
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void use(IPipeline& pipeline) final { glBindProgramPipeline(*static_cast<GLuint*>(pipeline.getHandler())); }

	void render(IMesh& mesh_, size_t instances) final {
		GLMeshImpl& mesh = static_cast<GLMeshImpl&>(mesh_);
		glBindVertexArray(mesh.getVAO());
		glDrawElementsInstanced(GL_TRIANGLES, mesh.getIndicesCount(), GL_UNSIGNED_INT, NULL, instances);
	}

	void flush() final {
		// TODO:
	}

private:
	SDL_Window* _window;
	SDL_GLContext _glContext;

	RenderOrder _renderOrder = RenderOrder::frontToBack;

	static void _loadGLAD() {
		static bool initialized = false;
		if (!initialized) {
			gladLoadGLLoader(SDL_GL_GetProcAddress);
			initialized = true;
		}
	}
};

std::unique_ptr<IRenderer> GLRenderer::create(Hydra::View::IView& view) {
	return std::unique_ptr<IRenderer>(new ::GLRendererImpl(view));
}

std::unique_ptr<IMesh> GLMesh::create(const std::string& file) {
	return std::unique_ptr<IMesh>(new ::GLMeshImpl(file));
}

std::shared_ptr<ITexture> GLTexture::createFromFile(const std::string& file) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(file));
}

std::shared_ptr<ITexture> GLTexture::createFromData(uint32_t width, uint32_t height, void* data) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(width, height, data));
}

std::shared_ptr<ITexture> GLTexture::createFromDataExt(const char* ext, void* data, uint32_t size) {
	return std::shared_ptr<ITexture>(new ::GLTextureImpl(ext, data, size));
}

void glDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	std::string sourceStr = "!UNKNOWN!";
	if (source == GL_DEBUG_SOURCE_API)
		sourceStr = "GL_DEBUG_SOURCE_API";
	else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		sourceStr = "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
	else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER)
		sourceStr = "GL_DEBUG_SOURCE_SHADER_COMPILER";
	else if (source == GL_DEBUG_SOURCE_THIRD_PARTY)
		sourceStr = "GL_DEBUG_SOURCE_THIRD_PARTY";
	else if (source == GL_DEBUG_SOURCE_APPLICATION)
		sourceStr = "GL_DEBUG_SOURCE_APPLICATION";
	else if (source == GL_DEBUG_SOURCE_OTHER)
		sourceStr = "GL_DEBUG_SOURCE_OTHER";

	std::string typeStr = "!UNKNOWN!";

	if (type == GL_DEBUG_TYPE_ERROR)
		typeStr = "GL_DEBUG_TYPE_ERROR";
	else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		typeStr = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
	else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		typeStr = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
	else if (type == GL_DEBUG_TYPE_PORTABILITY)
		typeStr = "GL_DEBUG_TYPE_PORTABILITY";
	else if (type == GL_DEBUG_TYPE_PERFORMANCE)
		typeStr = "GL_DEBUG_TYPE_PERFORMANCE";
	else if (type == GL_DEBUG_TYPE_MARKER)
		typeStr = "GL_DEBUG_TYPE_MARKER";
	else if (type == GL_DEBUG_TYPE_PUSH_GROUP)
		typeStr = "GL_DEBUG_TYPE_PUSH_GROUP";
	else if (type == GL_DEBUG_TYPE_POP_GROUP)
		typeStr = "GL_DEBUG_TYPE_POP_GROUP";
	else if (type == GL_DEBUG_TYPE_OTHER)
		typeStr = "GL_DEBUG_TYPE_OTHER";

	std::string severityStr = "!UNKNOWN!";

	if (severity == GL_DEBUG_SEVERITY_HIGH)
		severityStr = "GL_DEBUG_SEVERITY_HIGH";
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
		severityStr = "GL_DEBUG_SEVERITY_MEDIUM";
	else if (severity == GL_DEBUG_SEVERITY_LOW)
		severityStr = "GL_DEBUG_SEVERITY_LOW";
	else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		severityStr = "GL_DEBUG_SEVERITY_NOTIFICATION";

	fprintf(stdout, "[%s][%s][%d][%s] %s\n", sourceStr.c_str(), typeStr.c_str(), id, severityStr.c_str(), message);
}
