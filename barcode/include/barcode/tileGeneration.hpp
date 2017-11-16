#pragma once

#include <hydra/engine.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/io/meshloader.hpp>
#include <hydra/io/textureloader.hpp>
#include <hydra/world/blueprintloader.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/renderer/glshader.hpp>
#include <hydra/io/gltextureloader.hpp>
#include <hydra/io/glmeshloader.hpp>

#include <memory>
#include <imgui/imgui.h>
<<<<<<< HEAD
=======
#include <barcode/importermenu.hpp>
#include <barcode/exportermenu.hpp>
>>>>>>> master

#include <barcode/filetree.hpp>
#include <hydra/component/roomcomponent.hpp>

#include <hydra/world/blueprintloader.hpp>

#include <fstream>
#include <json.hpp>

class TileGeneration
{
public:
	int maxRooms = 1;
	std::shared_ptr<Hydra::Component::RoomComponent> roomGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
	bool** pathfindingMap;
	TileGeneration(std::string middleRoomPath);
	~TileGeneration();
<<<<<<< HEAD
=======

	int maxRooms = 4;

>>>>>>> master
private:
	std::vector<std::string> _roomFileNames;
	int _roomCounter = 0;

	void _setUpMiddleRoom(std::string middleRoomPath);
	void _createMapRecursivly(glm::ivec2 pos);
	void _insertPathFindingMap(glm::ivec2 room);
	void _setupGrid();
	void _obtainRoomFiles();
	glm::vec3 _gridToWorld(int x, int y);
	bool _checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r);
<<<<<<< HEAD
};
=======

};
>>>>>>> master
