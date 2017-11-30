#pragma once
#include <barcode/filetree.hpp>
#include <fstream>
#include <hydra/component/roomcomponent.hpp>
class PathingMapMenu{
public:
	PathingMapMenu();
	~PathingMapMenu();

	void render(bool &closeBool, float delta, int sizeX, int sizeY);
	
	//bool selected[32][32] = {false};
private:
};
