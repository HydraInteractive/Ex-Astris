#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

// NOTE: This GLRenderer depends on SDL aswell!
//       Maybe abstract way this in the future.
#include <SDL2/SDL.h>

using namespace Hydra::Renderer;

static void glDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

class GLRendererImpl final : public IRenderer {
public:
	GLRendererImpl(Hydra::View::IView& view) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		_glContext = SDL_GL_CreateContext(_window = static_cast<SDL_Window*>(view.getHandler()));
		_loadGLAD();

		glDebugMessageCallback(&glDebugLog, nullptr);
		glEnable(GL_DEBUG_OUTPUT);

		SDL_GL_SetSwapInterval(0);
	}

	~GLRendererImpl() final {
		SDL_GL_DeleteContext(_glContext);
	}

	void bind(Hydra::View::IRenderTarget& renderTarget) final {
		SDL_GL_MakeCurrent(_window, _glContext);
		glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.getID());
	}

	void clear(glm::vec4 clearColor) final {
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void use(IPipeline& pipeline) final { glBindProgramPipeline(*static_cast<GLuint*>(pipeline.getHandler())); }

private:
	SDL_Window* _window;
	SDL_GLContext _glContext;

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
