#pragma once
#include <barcode/filetree.hpp>
#include <fstream>
#include <hydra/component/roomcomponent.hpp>
class PathingMapMenu{
public:
	PathingMapMenu();
	~PathingMapMenu();

	void render(bool &closeBool, float delta);
private:
	int arr[ROOM_MAP_SIZE][ROOM_MAP_SIZE] = { 0 };
};
