#include <hydra/renderer/glrenderer.hpp>

#include <glad/glad.h>

// NOTE: This GLRenderer depends on SDL aswell!
//       Maybe abstract way this in the future.
#include <SDL2/SDL.h>

class GLRenderer final : public Hydra::Renderer::IRenderer {
public:
	GLRenderer(Hydra::View::IView* view) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		_glContext = SDL_GL_CreateContext(static_cast<SDL_Window*>(view->getHandler()));
		_loadGLAD();
	}

	~GLRenderer() final {
		SDL_GL_DeleteContext(_glContext);
	}

	void bind(Hydra::View::IRenderTarget* renderTarget) final {
		glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->getID());
	}

	void clear(glm::vec4 clearColor) final {
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

private:
	SDL_GLContext _glContext;

	static void _loadGLAD() {
		static bool initialized = false;
		if (!initialized) {
			gladLoadGLLoader(SDL_GL_GetProcAddress);
			initialized = true;
		}
	}

};

Hydra::Renderer::IRenderer* Hydra::Renderer::GLRenderer::create(Hydra::View::IView* view) {
	return static_cast<Hydra::Renderer::IRenderer*>(new ::GLRenderer(view));
}
