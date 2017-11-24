#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <hydra/world/blueprintloader.hpp>

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

#ifdef _WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/roomcomponent.hpp>

#define MAX_ENEMIES 4
#define PICKUP_CHANCE 40

class TileGeneration
{
public:
	std::shared_ptr<Hydra::Component::RoomComponent> roomGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
	bool** pathfindingMap = nullptr;
	int maxRooms = 1;
	int numberOfPlayers = 4;
	std::vector<glm::vec3> playerSpawns = std::vector<glm::vec3>();

	TileGeneration(std::string middleRoomPath);
	~TileGeneration();

	bool** buildMap();
private:
	enum { NORTH, EAST, SOUTH, WEST };
	std::vector<std::string> _roomFileNames = std::vector<std::string>();
	int _roomCounter = 0;

	void _setUpMiddleRoom(std::string middleRoomPath);
	void _createMapRecursivly(glm::ivec2 pos);
	void _insertPathFindingMap(glm::ivec2 room);
	void _obtainRoomFiles();
	void _randomizeRooms();
	bool _generatePlayerSpawnPoints();
	void _spawnEnemies(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
	void _spawnRandomEnemy(glm::vec3 pos);
	void _clearSpawnPoints();
	void _spawnPickUps(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
	void _spawnLight(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
	glm::quat _rotateRoom(std::shared_ptr<Hydra::Component::RoomComponent>& room);
	glm::vec3 _gridToWorld(int x, int y);
	bool _checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r);
};

