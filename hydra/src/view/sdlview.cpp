#include <hydra/view/sdlview.hpp>

#include <SDL2/SDL.h>

using namespace Hydra::View;

class SDLViewImpl final : public IView {
public:
	SDLViewImpl() {
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

		_size = glm::ivec2(1920, 1080);
		_wantToClose = false;

		_window = SDL_CreateWindow("SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _size.x, _size.y, SDL_WINDOW_OPENGL);
	}

	~SDLViewImpl() final {
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
	void* getHandler() final { return _window; }

	bool isClosed() final { return _wantToClose; }
	bool didChangeSize() final { return false; }

	// IRenderTarget

	uint32_t getID() final { return 0; } // Because 0 = Screen

	void finalize() final {
		SDL_GL_SwapWindow(_window);
	}

private:
	SDL_Window* _window;

	glm::ivec2 _size;

	bool _wantToClose;
};


std::unique_ptr<IView> SDLView::create() {
	return std::unique_ptr<IView>(new ::SDLViewImpl());
}
