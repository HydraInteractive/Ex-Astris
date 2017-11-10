#pragma once
#include <barcode/filetree.hpp>
#include <vector>
class PathingMapMenu : public FileTree {
public:
	PathingMapMenu();
	~PathingMapMenu();

	void saveMap();
	bool loadMap(std::string mapName);
	void render(bool &closeBool, float delta);
private:
	void clear();
	int outputMapNr;
};
