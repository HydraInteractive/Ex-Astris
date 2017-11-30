#pragma once

#include <hydra/engine.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/io/meshloader.hpp>
#include <hydra/io/textureloader.hpp>
#include <hydra/world/blueprintloader.hpp>


#include <memory>
#include <imgui/imgui.h>
#ifdef _WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif

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

namespace BarcodeServer {
	class TileGeneration {
	public:
		std::shared_ptr<Hydra::Component::RoomComponent> roomGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
		bool** pathfindingMap = nullptr;
		int maxRooms = 100;
		int roomCounter = 0;
		int numberOfPlayers = 4;
		int numberOfEnemies = 10; //Can be per room or for the whole map depending on if the _spawnEnemies function is run once per room or after the whole map is generated
		std::vector<glm::vec3> playerSpawns;

		TileGeneration(const std::string& middleRoomPath);
		~TileGeneration();

		bool** buildMap();
		std::shared_ptr<Hydra::World::Entity> mapentity;

	private:
		std::vector<std::string> _roomFileNames;

		enum { NORTH, EAST, SOUTH, WEST };
		glm::vec2 localXY;
		void _setUpMiddleRoom(const std::string& middleRoomPath);
		void _createMapRecursivly(const glm::ivec2& pos);
		void _insertPathFindingMap(const glm::ivec2& room, uint8_t rotation);
		void _obtainRoomFiles();
		void _randomizeRooms();
		void _spawnRandomizedEnemies(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
		void _spawnPickUps(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
		void _spawnLight(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
		glm::quat _rotateRoom(std::shared_ptr<Hydra::Component::RoomComponent>& room, uint8_t& rot);
		glm::vec3 _gridToWorld(int x, int y);
		bool _checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r);
	};
}
