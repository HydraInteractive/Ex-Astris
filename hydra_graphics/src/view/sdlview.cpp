// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the IView, using SDL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/view/sdlview.hpp>

#include <SDL2/SDL.h>

#include <hydra/renderer/uirenderer.hpp>

#include <glad/glad.h>

using namespace Hydra::View;

class SDLViewImpl final : public IView {
public:
	SDLViewImpl() {
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

		_size = glm::ivec2(1920, 1080);
		_wantToClose = false;

		_window = SDL_CreateWindow("SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _size.x, _size.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	}

	~SDLViewImpl() final {
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}

	// IView
	void update(Hydra::Renderer::IUIRenderer* uiRenderer) final {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (uiRenderer)
				uiRenderer->handleEvent(event);
			switch (event.type) {
			case SDL_QUIT:
				_wantToClose = true;
				break;

			case SDL_KEYDOWN:
				if (uiRenderer && uiRenderer->usingKeyboard())
					break;
				if (event.key.keysym.sym == SDLK_ESCAPE)
					_wantToClose = true;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					_size = glm::ivec2{event.window.data1, event.window.data2};
				break;
			default:
				break;
			}
		}
	}

	void show() final { SDL_ShowWindow(_window); }
	void hide() final {	SDL_HideWindow(_window); }
	void quit() final { _wantToClose = true; }

	void* getHandler() final { return _window; }

	bool isClosed() final { return _wantToClose; }
	bool didChangeSize() final { return false; }

	// ITexture
	void resize(glm::ivec2 size) final { SDL_SetWindowSize(_window, size.x, size.y); }

	glm::ivec2 getSize() final { return _size; }

	uint32_t getID() const final { return 0; } // Because 0 = Screen

	// IRenderTarget
	void finalize() final {
		SDL_GL_SwapWindow(_window);
		glFlush();
	}

private:
	SDL_Window* _window;

	glm::ivec2 _size;

	bool _wantToClose;
};


std::unique_ptr<IView> SDLView::create() {
	return std::unique_ptr<IView>(new ::SDLViewImpl());
}
