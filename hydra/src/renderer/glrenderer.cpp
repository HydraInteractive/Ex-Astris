#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

// NOTE: This GLRenderer depends on SDL aswell!
//       Maybe abstract way this in the future.
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <hydra/engine.hpp>
#include <hydra/ext/stacktrace.hpp>

using namespace Hydra::Renderer;

static void glDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

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

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&glDebugLog, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		SDL_GL_SetSwapInterval(0);

		glGenBuffers(1, &_modelMatrixBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
		glBufferData(GL_ARRAY_BUFFER, _modelMatrixSize, NULL, GL_DYNAMIC_DRAW);
	}

	~GLRendererImpl() final {
		glDeleteBuffers(1, &_modelMatrixBuffer);
		SDL_GL_DeleteContext(_glContext);
	}

	void render(Batch& batch) final {
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, batch.renderTarget->getID());
		const auto& size = batch.renderTarget->getSize();
		glViewport(0, 0, size.x, size.y);

		glClearColor(batch.clearColor.r, batch.clearColor.g, batch.clearColor.b, batch.clearColor.a);
		GLenum clearFlags = 0;
		clearFlags |= (batch.clearFlags & ClearFlags::color) == ClearFlags::color ? GL_COLOR_BUFFER_BIT : 0;
		clearFlags |= (batch.clearFlags & ClearFlags::depth) == ClearFlags::depth ? GL_DEPTH_BUFFER_BIT : 0;
		glClear(clearFlags);

		glUseProgram(*static_cast<GLuint*>(batch.pipeline->getHandler()));

		for (std::pair<const IMesh*, std::vector<glm::mat4>> kv : batch.objects) {
			auto& mesh = kv.first;

			size_t size = kv.second.size();
			const size_t maxPerLoop = _modelMatrixSize / sizeof(glm::mat4);
			for (size_t i = 0; i < size; i += maxPerLoop) {
				size_t amount = std::min(size - i, maxPerLoop);
				glBindBuffer(GL_ARRAY_BUFFER, _modelMatrixBuffer);
				glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &kv.second[i]);
				glBindVertexArray(mesh->getID());
				glDrawElementsInstanced(GL_TRIANGLES, mesh->getIndicesCount(), GL_UNSIGNED_INT, NULL, amount);
			}
		}
	}

	DrawObject* aquireDrawObject() final {
		DrawObject* drawObj;
		if (_inactiveDrawObjects.empty())
			drawObj = new DrawObject();
		else {
			drawObj = _inactiveDrawObjects.back();
			_inactiveDrawObjects.pop_back();
		}
		_activeDrawObjects.push_back(drawObj);
		return drawObj;
	}

	std::vector<DrawObject*> activeDrawObjects() final { return _activeDrawObjects; }

	void cleanup() final {
		auto isInactive = [this](DrawObject* drawObj) {
			if (drawObj->refCounter)
				return false;
			_inactiveDrawObjects.push_back(drawObj);
			return true;
		};

		_activeDrawObjects.erase(std::remove_if(_activeDrawObjects.begin(), _activeDrawObjects.end(), isInactive), _activeDrawObjects.end());
	}

	void* getModelMatrixBuffer() final { return static_cast<void*>(&_modelMatrixBuffer); }

private:
	SDL_Window* _window;
	SDL_GLContext _glContext;
	std::vector<DrawObject*> _activeDrawObjects;
	std::vector<DrawObject*> _inactiveDrawObjects;

	const size_t _modelMatrixSize = sizeof(glm::mat4) * 128; // max 128 mesh instances per draw call
	GLuint _modelMatrixBuffer;

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

void glDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if(id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	std::string sourceStr = "!UNKNOWN!";
	if (source == GL_DEBUG_SOURCE_API)
		sourceStr = "API";
	else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		sourceStr = "WINDOW_SYSTEM";
	else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER)
		sourceStr = "SHADER_COMPILER";
	else if (source == GL_DEBUG_SOURCE_THIRD_PARTY)
		sourceStr = "THIRD_PARTY";
	else if (source == GL_DEBUG_SOURCE_APPLICATION)
		sourceStr = "APPLICATION";
	else if (source == GL_DEBUG_SOURCE_OTHER)
		sourceStr = "OTHER";

	std::string typeStr = "!UNKNOWN!";

	if (type == GL_DEBUG_TYPE_ERROR)
		typeStr = "ERROR";
	else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		typeStr = "DEPRECATED_BEHAVIOR";
	else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		typeStr = "UNDEFINED_BEHAVIOR";
	else if (type == GL_DEBUG_TYPE_PORTABILITY)
		typeStr = "PORTABILITY";
	else if (type == GL_DEBUG_TYPE_PERFORMANCE)
		typeStr = "PERFORMANCE";
	else if (type == GL_DEBUG_TYPE_MARKER)
		typeStr = "MARKER";
	else if (type == GL_DEBUG_TYPE_PUSH_GROUP)
		typeStr = "PUSH_GROUP";
	else if (type == GL_DEBUG_TYPE_POP_GROUP)
		typeStr = "POP_GROUP";
	else if (type == GL_DEBUG_TYPE_OTHER)
		typeStr = "OTHER";

	std::string severityStr = "!UNKNOWN!";

	Hydra::LogLevel level;

	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		severityStr = "HIGH";
		level = Hydra::LogLevel::error;
	} else if (severity == GL_DEBUG_SEVERITY_MEDIUM){
		severityStr = "MEDIUM";
		level = Hydra::LogLevel::warning;
	} else if (severity == GL_DEBUG_SEVERITY_LOW) {
		severityStr = "LOW";
		level = Hydra::LogLevel::normal;
	} else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		severityStr = "NOTIFICATION";
		level = Hydra::LogLevel::verbose;
	}

	std::string stackTrace = Hydra::Ext::getStackTrace();

	Hydra::IEngine::getInstance()->log(level, "GL error: Source %s, Type: %s, ID: %d, Severity: %s\n%s%s%s", sourceStr.c_str(), typeStr.c_str(), id, severityStr.c_str(), message, stackTrace.length() ? "\n" : "", stackTrace.c_str());
}
