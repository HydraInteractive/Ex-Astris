#include <hydra/view/sdlview.hpp>

#include <SDL2/SDL.h>
#include <glad/glad.h>

class SDLView final : public Hydra::View::IView {
public:
	SDLView() {
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

		_size = glm::ivec2(1920, 1080);
		_wantToClose = false;

		_window = SDL_CreateWindow("SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _size.x, _size.y, SDL_WINDOW_OPENGL);
		_glContext = SDL_GL_CreateContext(_window);
		gladLoadGLLoader(SDL_GL_GetProcAddress);
	}

	~SDLView() final {
		SDL_GL_DeleteContext(_glContext);
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}

	// IView
	void update() final {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				_wantToClose = true;
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					_wantToClose = true;
				break;

			default:
				break;
			}
		}
	}

	void show() final { SDL_ShowWindow(_window); }
	void hide() final {	SDL_HideWindow(_window); }

	glm::ivec2 getSize() final { return _size; }

	bool isClosed() final { return _wantToClose; }
	bool didChangeSize() final { return false; }

	// IRenderTarget
	void begin() final {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void clear(glm::vec4 clearColor) final {
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void end() final {
		SDL_GL_SwapWindow(_window);
	}

private:
	SDL_Window* _window;
	SDL_GLContext _glContext;

	glm::ivec2 _size;

	bool _wantToClose;
};


Hydra::View::IView* Hydra::View::SDLView::create() {
	return static_cast<Hydra::View::IView*>(new ::SDLView());
}
