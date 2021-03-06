#include <server/tilegeneration.hpp>

#include <fstream>
#include <json.hpp>
#include <imgui/imgui.h>
#include <algorithm> //std::shuffle
#include <random> //std::default_random_engine
#include <chrono> //std::chrono::system_clock

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/pickupcomponent.hpp>
#include <hydra/component/textcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/spawnpointcomponent.hpp>
#include <hydra/component/spawnercomponent.hpp>
#include <hydra/component/networksynccomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
using world = Hydra::World::World;

using namespace BarcodeServer;

TileGeneration::TileGeneration(size_t maxRooms, const std::string& middleRoomPath, Hydra::Component::WeaponComponent::onShoot_f onRobotShoot, void* userdata, int level) : maxRooms(maxRooms), _onRobotShoot(onRobotShoot), _userdata(userdata) {
	_level = level;
	mapentity = world::newEntity("Map", world::root());
	_obtainRoomFiles();
	pathfindingMap = new bool*[WORLD_MAP_SIZE];
	for (int i = 0; i < WORLD_MAP_SIZE; i++) {
		pathfindingMap[i] = new bool[WORLD_MAP_SIZE];
		for (int j = 0; j < WORLD_MAP_SIZE; j++)
			pathfindingMap[i][j] = false;
	}
	_setUpMiddleRoom(middleRoomPath);
}

TileGeneration::~TileGeneration() {
	mapentity->dead = true;
	for (size_t x = 0; x < ROOM_GRID_SIZE; x++)
		for (size_t y = 0; y < ROOM_GRID_SIZE; y++)
			roomGrid[x][y].reset();

	for (size_t i = 0; i < WORLD_MAP_SIZE; i++)
		delete[] pathfindingMap[i];
	delete[] pathfindingMap;
}

void TileGeneration::buildMap() {
	if(_level < 2)
		_createMapRecursivly(glm::ivec2(ROOM_GRID_SIZE / 2, ROOM_GRID_SIZE / 2));
	else if (_level == 2) {
		//_setUpMiddleRoom("assets/BossRoom/Bossroom5.room");
	}
}

void TileGeneration::finalize() {
	_clearSpawnPoints();
}

std::string BarcodeServer::TileGeneration::getPathMapAsString()
{
	std::string map = "";
	for (int y = 0; y < WORLD_MAP_SIZE; y++)
	{
		for (int x = 0; x < WORLD_MAP_SIZE; x++)
		{
			map.append(std::to_string(pathfindingMap[x][y]));
		}
	}
	return map;
}

void TileGeneration::_createMapRecursivly(const glm::ivec2& pos) {
	//Random the rooms vector each time we're about to spawn a new room
	_randomizeRooms();

	//Go through all doors in the room of the tile, create a random room
	//That fits with the door, go through that room and so on, until the next
	//you get to a "end room". Then go back and continue for next door

	//Check if yTilePos or xTilePos == 0. If this is true, limit the rooms it can create
	//or we will go otuside the tile grid

	const int      nesw[4] = { NORTH, EAST, SOUTH, WEST };
	const glm::ivec2 offset[4] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

	for (size_t direction = 0; direction < sizeof(nesw) / sizeof(nesw[0]); direction++) {
		const int dir = nesw[direction];
		const int negDir = nesw[(direction + 2) % 4];
		if (roomGrid[pos.x][pos.y]->door[dir] && roomGrid[pos.x + offset[direction].x][pos.y + offset[direction].y] == nullptr) {
			bool placed = false;
			for (size_t i = 0; i < _roomFileNames.size() * 2 && !placed && roomCounter < maxRooms; i++) {
				auto loadedRoom = world::newEntity("Room", mapentity);

				BlueprintLoader::load(_roomFileNames[i / 2])->spawn(loadedRoom);
				auto roomC = loadedRoom->getComponent<Hydra::Component::RoomComponent>();
				uint8_t rot;
				glm::quat rotation = _rotateRoom(roomC, rot);
				roomC->rot = rot;

				if (roomC->door[negDir] && _checkAdjacents(pos.x + offset[direction].x, pos.y + offset[direction].y, roomC)) {
					deadSystem.tick(0);
					placed = true;
					auto t = loadedRoom->getComponent<Hydra::Component::TransformComponent>();
					t->position = _gridToWorld(pos.x + offset[direction].x, pos.y + offset[direction].y);
					t->scale = glm::vec3(1, 1, 1);
					t->rotation = rotation;
					roomGrid[pos.x + offset[direction].x][pos.y + offset[direction].y] = roomC;
					roomC->gridPosition = { pos.x + offset[direction].x, pos.y + offset[direction].y };
					_insertPathFindingMap(glm::ivec2(pos.x + offset[direction].x, pos.y + offset[direction].y), rot);
					//spawnPickUps();
					_spawnLight(t);

					int randomAlienSpawner = rand() % 101;
					int randomRobotSpawner = rand() % 101;
					if (randomAlienSpawner <= 3)
					{
						_createSpawner(loadedRoom, 1);
					}
					else if (randomRobotSpawner <= 2)
					{
						_createSpawner(loadedRoom, 2);
					}

					roomCounter++;
					_createMapRecursivly(glm::ivec2(pos.x + offset[direction].x, pos.y + offset[direction].y));
				}
				else
					loadedRoom->dead = true;
			}
		}
	}
	deadSystem.tick(0);
}

void TileGeneration::spawnDoors() {
	const int        nesw[4] = { NORTH, EAST, SOUTH, WEST };
	const char*      strNESW[4] = { "NORTH", "EAST", "SOUTH", "WEST" };
	const glm::ivec2 offset[4] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

	for (int y = 0; y < ROOM_GRID_SIZE; y++)
		for (int x = 0; x < ROOM_GRID_SIZE; x++) {
			auto& room = roomGrid[x][y];
			if (!room)
				continue;

			for (size_t direction = 0; direction < sizeof(nesw) / sizeof(nesw[0]); direction++) {
				auto dir = (direction + room->rot) % 4;
				if (!room->door[direction])
					continue;

				auto& roomOff = offset[dir];
				auto& gridOff = offset[direction];

				int nextDoorX = x + gridOff.x;
				int nextDoorY = y + gridOff.y;
				if (nextDoorX >= 0 && nextDoorX < ROOM_GRID_SIZE && nextDoorY >= 0 && nextDoorY < ROOM_GRID_SIZE) {
					auto& nextRoom = roomGrid[nextDoorX][nextDoorY];
					if (nextRoom) {
						if (nextRoom->door[(direction + 2) % 4])
							continue;
						else
							nextRoom->door[(direction + 2) % 4] = false;
					}
				}

				room->door[direction] = false;

				printf("\t\t\tAdding door\n");
				char tmp[64] = { 0 };
				snprintf(tmp, sizeof(tmp), "Door-%s", strNESW[direction]);

				auto doorBlock = world::newEntity(tmp, room->entityID);
				doorBlock->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Door.mATTIC");

				auto t = doorBlock->addComponent<Hydra::Component::TransformComponent>();
				t->position = { (ROOM_SIZE / 2) * roomOff.x, 3, (ROOM_SIZE / 2) * roomOff.y };
				t->scale = glm::vec3(1, 6, 8);
				t->rotation = glm::angleAxis(glm::radians((dir + 1) * 90.0f), glm::vec3(0, 1, 0));

				auto rgbc = doorBlock->addComponent<Hydra::Component::GhostObjectComponent>();
				rgbc->createBox(glm::vec3(1), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_WALL, glm::quat());
			}
		}
}

void TileGeneration::_setUpMiddleRoom(const std::string& middleRoomPath) {
	auto room = world::newEntity("Middle Room", mapentity);
	BlueprintLoader::load(middleRoomPath)->spawn(room);
	auto t = room->addComponent<Hydra::Component::TransformComponent>();
	t->position = _gridToWorld(ROOM_GRID_SIZE / 2, ROOM_GRID_SIZE / 2);
	t->scale = glm::vec3(1, 1, 1);
	auto roomC = room->getComponent<Hydra::Component::RoomComponent>();
	roomGrid[ROOM_GRID_SIZE / 2][ROOM_GRID_SIZE / 2] = roomC;
	roomC->gridPosition = { ROOM_GRID_SIZE / 2, ROOM_GRID_SIZE / 2 };
	_insertPathFindingMap(glm::ivec2(ROOM_GRID_SIZE / 2, ROOM_GRID_SIZE / 2), 0 /* No rotation */);
	_generatePlayerSpawnPoints();
	if(_level != 2)
		_clearSpawnPoints();
	//_spawnPickUps(room);
	_spawnLight(t);
}

void TileGeneration::_obtainRoomFiles() {
	//Get the files in order
	std::string path = "assets/room/";
	for (auto & p : std::experimental::filesystem::directory_iterator(path))
		_roomFileNames.push_back(p.path().string());

	_randomizeRooms();
}

void TileGeneration::_randomizeRooms() {
	//Randomize the list 2 times for extra randomness
	for (size_t k = 0; k < 2; k++)
		for (size_t i = 0; i < _roomFileNames.size(); i++) {
			int randomPos = rand() % _roomFileNames.size();
			std::swap(_roomFileNames[i], _roomFileNames[randomPos]);
		}
}

//Call after creating the first room to spawn players in the same room, call after creating the whole map to randomly spawn players across the whole map
//Returns false if not enough player spawn points were found
bool TileGeneration::_generatePlayerSpawnPoints() {
	//Get all spawnpoints
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities = std::vector<std::shared_ptr<Hydra::World::Entity>>();
	world::getEntitiesWithComponents<Hydra::Component::SpawnPointComponent, Hydra::Component::TransformComponent>(entities);

	//Randomize order
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(entities.begin(), entities.end(), std::default_random_engine(seed));
	for (size_t i = 0; i < entities.size() && playerSpawns.size() < numberOfPlayers; i++) {
		auto sp = entities[i]->getComponent<Hydra::Component::SpawnPointComponent>();
		if (sp->playerSpawn && !entities[i]->dead) {
			auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
			playerSpawns.push_back(t->getMatrix()[3]);
			entities[i]->dead = true;
		}
	}
	if (playerSpawns.size() < numberOfPlayers) {
		std::cout << "Tried to get " << numberOfPlayers << " player spawnpoints but could only find " << playerSpawns.size() << "." << std::endl;
		return false;
	}
	deadSystem.tick(0);
	return true;
}

void TileGeneration::spawnEnemies() {
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
	world::getEntitiesWithComponents<Hydra::Component::SpawnPointComponent, Hydra::Component::TransformComponent>(entities);
	//Randomize order
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(entities.begin(), entities.end(), std::default_random_engine(seed));

	size_t spawned = 0;
	for (size_t i = 0; i < entities.size() && spawned < numberOfEnemies; i++) {
		auto sp = entities[i]->getComponent<Hydra::Component::SpawnPointComponent>();
		if (sp->enemySpawn && !entities[i]->dead) {
			auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
			t->dirty = true;
			_spawnRandomEnemy(t->getMatrix()[3]);
			entities[i]->dead = true;
			spawned++;
		}
	}
	if (spawned < numberOfEnemies)
		std::cout << "Tried to spawn " << numberOfEnemies << " enemies but could only find " << spawned << " spawnpoints." << std::endl;
	deadSystem.tick(0);
}

void TileGeneration::_spawnRandomEnemy(glm::vec3 pos) {
	pos.y = pos.y + 2;
	int randNr = rand() % 3;
	printf("Spawning at (%.2f, %.2f, %.2f), AI: %d\n", pos.x, pos.y, pos.z, randNr);

	switch (randNr) {
	case 1: {
		auto alienEntity = world::newEntity("FastAlien1", world::root());
		alienEntity->addComponent<Hydra::Component::NetworkSyncComponent>();
		alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienFastModel2.mATTIC");
		auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		a->behaviour->setPathMap(pathfindingMap);
		a->damage = 4;
		a->behaviour->originalRange = 4.0f;
		a->behaviour->savedRange = a->behaviour->originalRange;
		a->radius = 1;

		auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 60;
		h->health = 60;

		auto w = alienEntity->addComponent<Hydra::Component::WeaponComponent>();
		w->meshType = 5;
		w->bulletSpread = 0.2f;
		w->bulletsPerShot = 1;
		w->damage = 4;
		w->bulletSize = 0.3;
		w->maxmagammo = 0;
		w->currmagammo = 0;
		w->maxammo = 0;
		w->userdata = _userdata;
		w->onShoot = _onRobotShoot;

		auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		m->movementSpeed = 10.0f;

		auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		t->position = pos;
		t->scale = glm::vec3{ 1,1,1 };

		auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(0.5f, 1.0f, 0.5f) * t->scale, glm::vec3(0, 1 * t->scale.y, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f, 0, 0, 0.6f, 1.0f);
		rgbc->createCapsuleY(0.5f, 1.0f * t->scale.y, glm::vec3(0, 2.6 * t->scale.y, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_HEAD, 10000, 0, 0, 0.0f, 0);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		rgbc->setAngularForce(glm::vec3(0));
	}
			break;
	case 2: {
		auto robotEntity = world::newEntity("Robot1", world::root());
		robotEntity->addComponent<Hydra::Component::NetworkSyncComponent>();
		robotEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/RobotModel2.mATTIC");
		auto a = robotEntity->addComponent<Hydra::Component::AIComponent>();
		a->behaviour = std::make_shared<RobotBehaviour>(robotEntity);
		a->behaviour->setPathMap(pathfindingMap);

		a->damage = 6;
		a->behaviour->originalRange = 18;
		a->behaviour->savedRange = a->behaviour->originalRange;
		a->radius = 1;

		auto h = robotEntity->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 70;
		h->health = 70;

		auto w = robotEntity->addComponent<Hydra::Component::WeaponComponent>();
		w->bulletSpread = 0.3f;
		w->fireRateRPM = 70;
		w->bulletsPerShot = 1;
		w->damage = 6;
		w->bulletSize = 0.3;
		w->maxmagammo = 0;
		w->currmagammo = 0;
		w->maxammo = 0;
		w->userdata = _userdata;
		w->onShoot = _onRobotShoot;

		auto m = robotEntity->addComponent<Hydra::Component::MovementComponent>();
		m->movementSpeed = 3.0f;

		auto t = robotEntity->addComponent<Hydra::Component::TransformComponent>();
		t->position = pos;
		t->scale = glm::vec3{ 1,1,1 };

		auto rgbc = robotEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(0.5f, 1.0f, 0.5f) * t->scale, glm::vec3(0, 1 * t->scale.y, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f, 0, 0, 0.6f, 1.0f);
		rgbc->createCapsuleY(0.5f, 1.0f * t->scale.y, glm::vec3(0, 2.6 * t->scale.y, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_HEAD, 10000, 0, 0, 0.0f, 0);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		rgbc->setAngularForce(glm::vec3(0));
	}
			break;
	default: {
		auto alienEntity = world::newEntity("SlowAlien1", world::root());
		alienEntity->addComponent<Hydra::Component::NetworkSyncComponent>();
		alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel2.mATTIC");
		auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);
		a->behaviour->setPathMap(pathfindingMap);

		a->damage = 4;
		a->behaviour->originalRange = 4.0f;
		a->behaviour->savedRange = a->behaviour->originalRange;
		a->radius = 1;

		auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 80;
		h->health = 80;

		auto w = alienEntity->addComponent<Hydra::Component::WeaponComponent>();
		w->meshType = 5;
		w->bulletSpread = 0.2f;
		w->bulletsPerShot = 1;
		w->damage = 4;
		w->bulletSize = 0.3;
		w->maxmagammo = 0;
		w->currmagammo = 0;
		w->maxammo = 0;
		w->userdata = _userdata;
		w->onShoot = _onRobotShoot;

		auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		m->movementSpeed = 5.0f;

		auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		t->position = pos;
		t->scale = glm::vec3{ 1,1,1 };

		auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(0.5f, 1.0f, 0.5f) * t->scale, glm::vec3(0, 1 * t->scale.y, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f, 0, 0, 0.6f, 1.0f);
		rgbc->createCapsuleY(0.5f, 1.0f * t->scale.y, glm::vec3(0, 2.6 * t->scale.y, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_HEAD, 10000, 0, 0, 0.0f, 0);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		rgbc->setAngularForce(glm::vec3(0));
	}
			 break;
	}
}

//TODO: Randomize spawners

void TileGeneration::_createSpawner(std::shared_ptr<Hydra::World::Entity>& room, int id) {
	glm::vec3 pos;
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
	for (auto entid : room->children)
	{
		if (world::getEntity(entid)->hasComponent<Hydra::Component::SpawnPointComponent>())
		{
			entities.push_back(world::getEntity(entid));
		}
	}

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(entities.begin(), entities.end(), std::default_random_engine(seed));

	for (size_t i = 0; i < entities.size(); i++) {
		auto sp = entities[i]->getComponent<Hydra::Component::SpawnPointComponent>();
		if (sp->enemySpawn && !entities[i]->dead) {
			auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
			t->dirty = true;
			pos = t->getMatrix()[3];
			entities[i]->dead = true;
		}

		if (sp->enemySpawn && !entities[i]->dead)
		{
			entities[i]->dead = true;
		}
	}

	if (id == 1) {
		auto alienSpawner = world::newEntity("SpawnerAlien1", world::root());
		alienSpawner->addComponent<Hydra::Component::NetworkSyncComponent>();
		alienSpawner->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/Spawner.mATTIC");
		auto sa = alienSpawner->addComponent<Hydra::Component::SpawnerComponent>();
		sa->spawnerID = Hydra::Component::SpawnerType::AlienSpawner;

		auto h = alienSpawner->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 50;
		h->health = 50;

		auto t = alienSpawner->addComponent<Hydra::Component::TransformComponent>();
		t->position.x = pos.x;
		t->position.y = 0.8;
		t->position.z = pos.z;
		float randDirX = ((float)rand() / (float)(RAND_MAX)) * (2.0f*3.14f);
		float randDirY = ((float)rand() / (float)(RAND_MAX)) * (2.0f*3.14f);
		t->rotation = glm::angleAxis(atan2(randDirX, randDirY), glm::vec3(0, 1, 0));
		t->scale = glm::vec3{ 1,1,1 };

		{
			auto pE = Hydra::World::World::newEntity("Spawner Collision Particle Spawner", alienSpawner->id);

			pE->addComponent<MeshComponent>()->loadMesh("PARTICLEQUAD");

			auto pETC = pE->addComponent<TransformComponent>();
			pETC->position.y = 1.0;

			auto pEPC = pE->addComponent<Hydra::Component::ParticleComponent>();
			pEPC->delay = 1.0f / 10.0f;
			pEPC->accumulator = 2 * 5.0f;
			pEPC->tempVelocity = glm::vec3(6.0f, 6.0f, 6.0f);
			pEPC->behaviour = Hydra::Component::ParticleComponent::EmitterBehaviour::SpawnerBeam;
			pEPC->texture = Hydra::Component::ParticleComponent::ParticleTexture::Spawner;
		}

		auto rgbc = alienSpawner->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(2.0f, 0.8f, 2.0f) * t->scale, glm::vec3(0, -0.2, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_SPAWNER, 0.0f,
			0, 0, 0.6f, 1.0f);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		rgbc->setAngularForce(glm::vec3(0));
	}
	else if (id == 2) {
		auto robotSpawner = world::newEntity("SpawnerRobot1", world::root());
		robotSpawner->addComponent<Hydra::Component::NetworkSyncComponent>();
		robotSpawner->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/Spawner.mATTIC");
		auto sa = robotSpawner->addComponent<Hydra::Component::SpawnerComponent>();
		sa->spawnerID = Hydra::Component::SpawnerType::RobotSpawner;

		auto h = robotSpawner->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 50;
		h->health = 50;

		auto t = robotSpawner->addComponent<Hydra::Component::TransformComponent>();
		t->position.x = pos.x;
		t->position.y = 0.8;
		t->position.z = pos.z;
		float randDirX = ((float)rand() / (float)(RAND_MAX)) * (2.0f*3.14f);
		float randDirY = ((float)rand() / (float)(RAND_MAX)) * (2.0f*3.14f);
		t->rotation = glm::angleAxis(atan2(randDirX, randDirY), glm::vec3(0, 1, 0));
		t->scale = glm::vec3{ 1,1,1 };

		{
			auto pE = Hydra::World::World::newEntity("Spawner Collision Particle Spawner", robotSpawner->id);

			pE->addComponent<MeshComponent>()->loadMesh("PARTICLEQUAD");

			auto pETC = pE->addComponent<TransformComponent>();
			pETC->position.y = 1.0;

			auto pEPC = pE->addComponent<Hydra::Component::ParticleComponent>();
			pEPC->delay = 1.0f / 10.0f;
			pEPC->accumulator = 2 * 5.0f;
			pEPC->tempVelocity = glm::vec3(6.0f, 6.0f, 6.0f);
			pEPC->behaviour = Hydra::Component::ParticleComponent::EmitterBehaviour::SpawnerBeam;
			pEPC->texture = Hydra::Component::ParticleComponent::ParticleTexture::Spawner;
		}

		auto rgbc = robotSpawner->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(2.0f, 0.8f, 2.0f) * t->scale, glm::vec3(0, -0.2, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_SPAWNER, 0.0f,
			0, 0, 0.6f, 1.0f);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		rgbc->setAngularForce(glm::vec3(0));
	}
}

void TileGeneration::_clearSpawnPoints() {
	//Get all spawnpoints
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities = std::vector<std::shared_ptr<Hydra::World::Entity>>();
	world::getEntitiesWithComponents<Hydra::Component::SpawnPointComponent, Hydra::Component::TransformComponent>(entities);

	for (auto e : entities)
		e->dead = true;
	deadSystem.tick(0);
}

void TileGeneration::_insertPathFindingMap(const glm::ivec2& room, uint8_t rot) {
	// rot = 0 -> 0
	// rot = 1 -> 90
	// rot = 2 -> 180
	// rot = 3 -> 270

	auto roomC = roomGrid[room.x][room.y];
	const int x = room.x * ROOM_MAP_SIZE;
	const int y = room.y * ROOM_MAP_SIZE;
	switch (rot) {
	case 0:
		for (int localX = 0; localX < ROOM_MAP_SIZE; localX++)
			for (int localY = 0; localY < ROOM_MAP_SIZE; localY++)
				pathfindingMap[x + localX][y + localY] = roomC->localMap[localX][localY];
		break;
	case 1:
		for (int localX = 0; localX < ROOM_MAP_SIZE; localX++)
			for (int localY = 0; localY < ROOM_MAP_SIZE; localY++)
				pathfindingMap[x + localX][y + localY] = roomC->localMap[ROOM_MAP_SIZE - 1 - localY][localX];
		break;
	case 2:
		for (int localX = 0; localX < ROOM_MAP_SIZE; localX++)
			for (int localY = 0; localY < ROOM_MAP_SIZE; localY++)
				pathfindingMap[x + localX][y + localY] = roomC->localMap[ROOM_MAP_SIZE - 1 - localX][ROOM_MAP_SIZE - 1 - localY];
		break;
	case 3:
		for (int localX = 0; localX < ROOM_MAP_SIZE; localX++)
			for (int localY = 0; localY < ROOM_MAP_SIZE; localY++)
				pathfindingMap[x + localX][y + localY] = roomC->localMap[localY][ROOM_MAP_SIZE - 1 - localX];
		break;
	default:
		break;
	}
}
//This function should be called once per room
void TileGeneration::spawnPickUps()
{
	for (int x = 0; x < ROOM_GRID_SIZE; x++) {
		for (int y = 0; y < ROOM_GRID_SIZE; y++) {
			int randomChance = rand() % 100 + 1;

			if (randomChance < (int)PICKUP_CHANCE) {
				auto room = roomGrid[x][y];

				if (room) {
					for (auto id : world::getEntity(room->entityID)->children)
					{
						int otherSpawner = rand() % 101;
						if (otherSpawner <= 70)
						{
							if (world::getEntity(id)->hasComponent<Hydra::Component::SpawnPointComponent>())
							{
								if (!world::getEntity(id)->dead) {
									auto pos = world::getEntity(id)->getComponent<Hydra::Component::TransformComponent>();

									auto pickUpEntity = world::newEntity("PickUp", world::root());
									pickUpEntity->addComponent<NetworkSyncComponent>();
									auto t = pickUpEntity->addComponent<Hydra::Component::TransformComponent>();
									t->position = { pos->position.x + room->gridPosition.x * ROOM_SIZE + ROOM_SIZE / 2, 3, pos->position.z + room->gridPosition.y * ROOM_SIZE + ROOM_SIZE / 2 };

									pickUpEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Lock.mATTIC");

									auto pickUpC = pickUpEntity->addComponent<Hydra::Component::PickUpComponent>();

									auto rgbc = pickUpEntity->addComponent<Hydra::Component::RigidBodyComponent>();
									rgbc->createBox(glm::vec3(2.0f, 1.5f, 1.7f), glm::vec3(0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PICKUP_OBJECT, 10);
									rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);

									auto lc = pickUpEntity->addComponent<Hydra::Component::LifeComponent>();
									lc->health = lc->maxHP;
								}
								break;
							}
						}
					}
				}
			}
			deadSystem.tick(0);
		}
	}
}

void TileGeneration::_spawnLight(std::shared_ptr<Hydra::Component::TransformComponent>& roomTransform) {
#define frand() (float(rand()) / RAND_MAX)
	if (_level < 2) {
	auto pl = world::newEntity("Pointlight-GENERATED", roomTransform->entityID);
	pl->addComponent<Hydra::Component::TransformComponent>();
	auto t = pl->getComponent<Hydra::Component::TransformComponent>();
	t->position.x = 0;
	t->position.y = 7;
	t->position.z = 0;
	auto lc = pl->addComponent<Hydra::Component::PointLightComponent>();
	lc->color = glm::vec3(frand() * 2, frand() * 2, frand() * 2);
	}
	else {
		auto pl = world::newEntity("Pointlight-GENERATED", roomTransform->entityID);
		auto t = pl->addComponent<Hydra::Component::TransformComponent>();
		t->position.x = 0;
		t->position.y = 50;
		t->position.z = 0;
		auto lc = pl->addComponent<Hydra::Component::PointLightComponent>();
		lc->color = glm::vec3(20);
	}
#undef frand
}

glm::quat TileGeneration::_rotateRoom(std::shared_ptr<Hydra::Component::RoomComponent>& room, uint8_t& rot) {
	//glm::angleAxis(rotation, axis(y in your case));

	glm::quat rotation;

	rot = rand() % 4;

	if (rot == 0)
		rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0, 1, 0));
	else if (rot == 1) {
		decltype(room->door) doors = {
			room->door[EAST],
			room->door[SOUTH],
			room->door[WEST],
			room->door[NORTH]
		};
		memcpy(room->door, doors, sizeof(doors));
		decltype(room->openWalls) openWalls = {
			room->openWalls[EAST],
			room->openWalls[SOUTH],
			room->openWalls[WEST],
			room->openWalls[NORTH]
		};
		memcpy(room->openWalls, openWalls, sizeof(openWalls));

		rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0));
	}
	else if (rot == 2) {
		decltype(room->door) doors = {
			room->door[SOUTH],
			room->door[WEST],
			room->door[NORTH],
			room->door[EAST]
		};
		memcpy(room->door, doors, sizeof(doors));
		decltype(room->openWalls) openWalls = {
			room->openWalls[SOUTH],
			room->openWalls[WEST],
			room->openWalls[NORTH],
			room->openWalls[EAST]
		};
		memcpy(room->openWalls, openWalls, sizeof(openWalls));

		rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 1, 0));
	}
	else if (rot == 3) {
		decltype(room->door) doors = {
			room->door[WEST],
			room->door[NORTH],
			room->door[EAST],
			room->door[SOUTH]
		};
		memcpy(room->door, doors, sizeof(doors));
		decltype(room->openWalls) openWalls = {
			room->openWalls[WEST],
			room->openWalls[NORTH],
			room->openWalls[EAST],
			room->openWalls[SOUTH]
		};
		memcpy(room->openWalls, openWalls, sizeof(openWalls));

		rotation = glm::angleAxis(glm::radians(270.0f), glm::vec3(0, 1, 0));
	}

	return rotation;
}

glm::vec3 TileGeneration::_gridToWorld(int x, int y) {
	float xPos = (x + 0.5f) * ROOM_SIZE;
	float yPos = (y + 0.5f) * ROOM_SIZE;

	return glm::vec3(xPos, 0, yPos);
}

bool TileGeneration::_checkAdjacents(int x, int y, std::shared_ptr<Hydra::Component::RoomComponent>& r) {
	if (r->door[NORTH]) {
		if (y < 1)
			return false;
		if (auto rr = roomGrid[x][y - 1]; rr && rr->door[SOUTH] == false)
			return false;
	}
	if (r->door[EAST]) {
		if (x >= ROOM_GRID_SIZE - 1)
			return false;
		if (auto rr = roomGrid[x + 1][y]; rr && rr->door[WEST] == false)
			return false;
	}
	if (r->door[SOUTH]) {
		if (y >= ROOM_GRID_SIZE - 1)
			return false;
		if (auto rr = roomGrid[x][y + 1]; rr && rr->door[NORTH] == false)
			return false;
	}
	if (r->door[WEST]) {
		if (x < 1)
			return false;
		if (auto rr = roomGrid[x - 1][y]; rr && rr->door[EAST] == false)
			return false;
	}
	return true;
}
