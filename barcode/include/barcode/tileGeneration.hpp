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
#ifdef _WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <barcode/importermenu.hpp>
#include <barcode/exportermenu.hpp>

#include <barcode/filetree.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/pickupcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>
#include <hydra/component/textcomponent.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <hydra/world/blueprintloader.hpp>

#include <fstream>
#include <json.hpp>

#define MAX_ENEMIES 4
#define PICKUP_CHANCE 40

class TileGeneration
{
public:
	std::shared_ptr<Hydra::Component::RoomComponent> roomGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
	bool** pathfindingMap = nullptr;
	bool** buildMap();
	TileGeneration(std::string middleRoomPath);
	~TileGeneration();

	int maxRooms = 1;

private:
	std::vector<std::string> _roomFileNames;
	int _roomCounter = 0;

	enum { NORTH, EAST, SOUTH, WEST };
	glm::vec2 localXY;
	void _setUpMiddleRoom(std::string middleRoomPath);
	void _createMapRecursivly(glm::ivec2 pos);
	void _insertPathFindingMap(glm::ivec2 room);
	void _obtainRoomFiles();
	void _randomizeRooms();
	void _spawnRandomizedEnemies(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
	void _spawnPickUps(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
	void _spawnLight(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
	glm::quat _rotateRoom(std::shared_ptr<Hydra::Component::RoomComponent>& room);
	glm::vec3 _gridToWorld(int x, int y);
	bool _checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r);
};

