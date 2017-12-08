#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

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
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/system/deadsystem.hpp>
#define PICKUP_CHANCE 100

namespace BarcodeServer {
	class TileGeneration {
	public:
		std::shared_ptr<Hydra::Component::RoomComponent> roomGrid[ROOM_GRID_SIZE][ROOM_GRID_SIZE];
		bool** pathfindingMap = nullptr;
		size_t maxRooms = 100;
		size_t roomCounter = 0;
		size_t numberOfPlayers = 4;
		size_t numberOfEnemies = 50; //Can be per room or for the whole map depending on if the _spawnEnemies function is run once per room or after the whole map is generated
		std::vector<glm::vec3> playerSpawns = std::vector<glm::vec3>();

		TileGeneration(const std::string& middleRoomPath, Hydra::Component::WeaponComponent::onShoot_f onRobotShoot, void* userdata);
		~TileGeneration();

		bool** buildMap();
		std::shared_ptr<Hydra::World::Entity> mapentity = nullptr;

	private:
		enum { NORTH, EAST, SOUTH, WEST };
		Hydra::Component::WeaponComponent::onShoot_f _onRobotShoot;
		void* _userdata;

		std::vector<std::string> _roomFileNames;
		Hydra::System::DeadSystem deadSystem;

		void _setUpMiddleRoom(const std::string& middleRoomPath);
		void _createMapRecursivly(const glm::ivec2& pos);
		void _insertPathFindingMap(const glm::ivec2& room, uint8_t rotation);
		void _obtainRoomFiles();
		void _randomizeRooms();
		bool _generatePlayerSpawnPoints();
		void _spawnEnemies();
		void _spawnRandomEnemy(glm::vec3 pos);
		void _createSpawner(glm::vec3 pos);
		void _clearSpawnPoints();
		void _spawnPickUps(std::shared_ptr<Hydra::World::Entity>& room);
		void _spawnLight(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform);
		glm::quat _rotateRoom(std::shared_ptr<Hydra::Component::RoomComponent>& room, uint8_t& rot);
		glm::vec3 _gridToWorld(int x, int y);
		bool _checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r);
	};
}