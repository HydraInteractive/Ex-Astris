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
#include <filesystem>	//Git gud
#include <barcode/importermenu.hpp>
#include <barcode/exportermenu.hpp>

#include <barcode/filetree.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>

#include <hydra/world/blueprintloader.hpp>

#include <fstream>
#include <json.hpp>
#define GRID_SIZE 5
#define ROOM_SIZE 34
#define MAX_ENEMIES 4

class TileGeneration
{
public:
	std::shared_ptr<Hydra::Component::RoomComponent> grid[GRID_SIZE][GRID_SIZE];

	TileGeneration(std::string middleRoomPath);
	~TileGeneration();

	int maxRooms = 25;

private:
	std::vector<std::string> _roomFileNames;
	int _roomCounter = 0;


	glm::vec2 localXY;
	void _setUpMiddleRoom(std::string middleRoomPath);
	void _createMapRecursivly(glm::ivec2 pos);
	void _setupGrid();
	void _obtainRoomFiles();
	void _randomizeRooms();
	void _spawnRandomizedEnemies(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
	glm::vec3 _gridToWorld(int x, int y);
	bool _checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r);
	
};
