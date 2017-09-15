#include <hydra/io/input.hpp>
#include <SDL2/SDL.h>
#include <hydra/engine.hpp>

Input::Input() {
	_kbState = SDL_GetKeyboardState(&_keyCount);
	_lockMouseCenter = false;
}

Input::~Input() {

}

void Input::setWindowSize(int x, int y) {
	_sizeX = x;
	_sizeY = y;
}

void Input::update(){
	auto& engine = Hydra::IEngine::getInstance();
	
	int x, y;
	int buttonState = SDL_GetMouseState(&x, &y);

	if (getKey(SDL_SCANCODE_L))
		_lockMouseCenter = true;

	if (_lockMouseCenter) {
		SDL_WarpMouseInWindow(NULL, _sizeX / 2, _sizeY / 2);
		_xyDiff = glm::ivec2(_sizeX / 2 - x, _sizeY / 2 - y);

		SDL_ShowCursor(false);
	}
	//else
		//SDL_ShowCursor(true);
		
}
