#pragma once

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <hydra/ext/api.hpp>

class HYDRA_API Input {
private:
	const uint8_t* _kbState = NULL;
	int _keyCount;
	bool _lockMouseCenter;
	int _sizeX, _sizeY;
	
	glm::ivec2 _xyDiff;


public:
	Input();
	~Input();

	bool getKey(SDL_Scancode key) { return _kbState[key]; }
	void setWindowSize(int x, int y);
	void update();
	
};