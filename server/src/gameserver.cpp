#include <server/gameserver.hpp>
#include <server/packets.hpp>
#include <server/tilegeneration.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/networksynccomponent.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/spawnercomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/pickupcomponent.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <glm/glm.hpp>

#ifdef __linux__
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#error DERP;
#endif

using namespace BarcodeServer;
using namespace Hydra::Network;

using world = Hydra::World::World;

enum Tile {
	Void = 0,
	Air,
	RoomContent,
	Wall,
	Portal,
	MAX_COUNT
};
static const glm::ivec3 colors[Tile::MAX_COUNT]{
	{0xFF, 0xFF, 0xFF},
	{0x3F, 0x3F, 0x3F},
	{0xFF, 0xFF, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0xFF, 0xFF}
};

static void mySleep(int sleepMs) {
#ifdef __linux__
	usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#elif defined(_WIN32)
	Sleep(sleepMs);
#else
#error DERP;
#endif
}

GameServer::GameServer() {}

GameServer::~GameServer() { quit(); }

bool GameServer::initialize(int port) {
	if (!this->_server) {
		this->_server = new Server();
		_lastTime = std::chrono::high_resolution_clock::now();
		_packetDelay = 0;
		if (this->_server->initialize(port)) {
			printf("I am a scurb\n");
			printf("Server started on port %d.\n", port);

			return true;
		}
		delete this->_server;
		return false;
	}
	return true;
}

void GameServer::start() {
	auto floor = world::newEntity("Floor", world::root());
	auto transf = floor->addComponent<Hydra::Component::TransformComponent>();
	transf->position = glm::vec3(0, 0, 0);
	auto rgbcf = floor->addComponent<Hydra::Component::RigidBodyComponent>();
	rgbcf->createStaticPlane(glm::vec3(0, 1, 0), 0, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_FLOOR
		, 0, 0, 0, 0.6f, 0);
	floor->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Floor_v2.mATTIC");

	level = 1;
	_makeWorld();


	printf("\n\n\n\n");
	printf("================SERVER INFO================\n");
	printf("Server is now ready for players!\n");
	printf("===========================================\n");
}

void GameServer::_spawnBoss() {
	//Boss
	{
		//Alien
		{
			auto BossAlien = world::newEntity("Boss Alien", world::root());
			_bossID = BossAlien->id;
			BossAlien->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/BossAlienModel.mATTIC");
			BossAlien->addComponent<Hydra::Component::NetworkSyncComponent>();

			auto a = BossAlien->addComponent<Hydra::Component::AIComponent>();
			a->behaviour = std::make_shared<StationaryBoss>(BossAlien);
						
			auto t = BossAlien->addComponent<Hydra::Component::TransformComponent>();
			t->position = glm::vec3(45 + 150, 26, 3 + 150);
			t->rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));

			auto l = BossAlien->addComponent<Hydra::Component::LifeComponent>();
			l->maxHP = 3000;
			l->health = 3000;

			auto rgbc = BossAlien->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createBox(glm::vec3(8.0f, 2.5f, 8.0f) * t->scale, glm::vec3(0, 0, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 0.0f,
				0, 0, 0.6f, 1.0f);
			rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableSimulation);
			rgbc->setAngularForce(glm::vec3(0));
		}
		///Stationary Mech
		//{
		//	auto BossMech = world::newEntity("BossMech", world::root());
		//	BossMech->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/BossStationaryMechModel.mATTIC");
		//
		//	auto a = BossMech->addComponent<Hydra::Component::AIComponent>();
		//	a->behaviour = std::make_shared<StationaryBoss>(BossMech);
		//	a->damage = 7;
		//	a->behaviour->originalRange = 20;
		//	a->radius = 1;
		//
		//	auto t = BossMech->addComponent<Hydra::Component::TransformComponent>();
		//	t->position = glm::vec3(51, 45, 0);
		//	t->rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));
		//
		//	auto l = BossMech->addComponent<Hydra::Component::LifeComponent>();
		//	l->maxHP = 1000.0f;
		//	l->health = 1000.0f;
		//
		//	auto m = BossMech->addComponent<Hydra::Component::MovementComponent>();
		//	m->movementSpeed = 0.0f;
		//
		//	auto rgbc = BossMech->addComponent<Hydra::Component::RigidBodyComponent>();
		//	rgbc->createBox(glm::vec3(10.0f, 16.0f, 10.0f) * t->scale, glm::vec3(0, 0, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 1000000.0f,
		//		100, 1000, 0.6f, 1.0f);
		//	rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		//	rgbc->setAngularForce(glm::vec3(0));
		//}
		///Arms
		{
			for (int i = 0; i < 2; i++) {
				{
					auto bossEntity = world::newEntity("Upper BossArm", world::root());
					bossEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/BossUpperArmModel.mATTIC");
					auto t = bossEntity->addComponent<Hydra::Component::TransformComponent>();
					t->rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));
					if (i == 0) {
						t->position = glm::vec3(210, 30, 24 + 150);
						t->scale = glm::vec3{ 1,1,1 };
					}
					else {
						t->position = glm::vec3(210, 30, -24 + 150);
						t->scale = glm::vec3{ -1,1,-1 };
					}

					bossEntity->addComponent<Hydra::Component::NetworkSyncComponent>();

					auto rgbc = bossEntity->addComponent<Hydra::Component::RigidBodyComponent>();
					rgbc->createBox(glm::vec3(8.0f, 2.5f, 8.0f) * glm::vec3(1), glm::vec3(0, 0, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
						0, 0, 0.6f, 1.0f);
					rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableSimulation);
					rgbc->setAngularForce(glm::vec3(0));
				}
				{
					auto bossEntity = world::newEntity("Lower BossArm", world::root());
					bossEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/BossLowerArmModel.mATTIC");
					bossEntity->addComponent<Hydra::Component::NetworkSyncComponent>();

					auto a = bossEntity->addComponent<Hydra::Component::AIComponent>();
					a->behaviour = std::make_shared<BossArm>(bossEntity);
					a->damage = 7;
					a->behaviour->originalRange = 20;
					a->radius = 1;

					auto w = bossEntity->addComponent<Hydra::Component::WeaponComponent>();
					w->ammoPerShot = 1;
					w->bulletSize = 2.0f;
					w->damage = 15;
					w->maxmagammo = 0;
					w->currmagammo = 0;
					w->maxammo = 0;
					w->fireRateTimer = 1;
					w->color[0] = 1.0f;
					w->glow = true;
					w->glowIntensity = 1.0f;
					w->userdata = _tileGeneration->_userdata;
					w->onShoot = _onRobotShoot;

					auto t = bossEntity->addComponent<Hydra::Component::TransformComponent>();
					if (i == 0) {
						t->position = glm::vec3(210, 30, 40 + 150);
						t->scale = glm::vec3{ 1,1,1 };
					}
					else {
						t->position = glm::vec3(210, 30, -40 + 150);
						t->scale = glm::vec3{ 1,1,1 };
					}

					auto l = bossEntity->addComponent<Hydra::Component::LifeComponent>();
					l->maxHP = 10000000;
					l->health = 10000000;

					//auto l = bossEntity->addComponent<Hydra::Component::LifeComponent>();
					//l->maxHP = 150;
					//l->health = 150;

					//auto t = bossEntity->addComponent<Hydra::Component::TransformComponent>();
					//t->position = glm::vec3(40 + 150, 20, 40 + 150);
					//t->scale = glm::vec3{ 1,1,1 };
					//t->rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));
					//
					//auto rgbc = bossEntity->addComponent<Hydra::Component::RigidBodyComponent>();
					//rgbc->createBox(glm::vec3(8.0f, 2.5f, 8.0f) * t->scale, glm::vec3(0, 0, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
					//	0, 0, 0.6f, 1.0f);
					//rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableSimulation);
					//rgbc->setAngularForce(glm::vec3(0));
					//
					//auto m = bossEntity->addComponent<Hydra::Component::MovementComponent>();
					//m->movementSpeed = 25.0f;
				}
			}
		}

		{
			//Hands
			for (int i = 0; i < 2; i++) {
				auto bossEntity = world::newEntity("BossHand1", world::root());
				bossEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/BossHandModel2.mATTIC");
				auto a = bossEntity->addComponent<Hydra::Component::AIComponent>();
				a->behaviour = std::make_shared<BossHand_Left>(bossEntity);
				//a->behaviour->setPathMap(_pathfindingMap);

				a->damage = 7;
				a->behaviour->originalRange = 20;
				a->radius = 1;

				bossEntity->addComponent<Hydra::Component::NetworkSyncComponent>();
				auto t = bossEntity->addComponent<Hydra::Component::TransformComponent>();
				//Set different positions for other hand
				if (i == 0) {
					t->position = glm::vec3(30 + 150, 30, 40 + 150);
					t->scale = glm::vec3{ 1,1,1 };
				}
				else {
					a->behaviour->handID = 1;
					t->position = glm::vec3(30 + 150, 30, -40 + 150);
					t->scale = glm::vec3{ -1,1,-1 };
				}

				auto h = bossEntity->addComponent<Hydra::Component::LifeComponent>();
				h->maxHP = 2000000000;
				h->health = 2000000000;

				auto w = bossEntity->addComponent<Hydra::Component::WeaponComponent>();
				w->bulletSpread = 1.0f;
				w->fireRateRPM = 5000000;
				w->fireRateTimer = 0.1f;
				w->bulletsPerShot = 1;
				w->damage = 7;

				w->maxmagammo = 0;
				w->currmagammo = 0;
				w->maxammo = 0;
				w->color[0] = 1;
				w->color[1] = 0;
				w->color[2] = 1;
				w->color[3] = 0.5f;
				w->userdata = _tileGeneration->_userdata;
				w->onShoot = _onRobotShoot;

				auto m = bossEntity->addComponent<Hydra::Component::MovementComponent>();
				m->movementSpeed = 50.0f;
	
				auto rgbc = bossEntity->addComponent<Hydra::Component::RigidBodyComponent>();
				rgbc->createBox(glm::vec3(8.0f, 2.5f, 8.0f) * glm::vec3(1), glm::vec3(0, 0, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
					0, 0, 0.6f, 1.0f);
				rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
				rgbc->setAngularForce(glm::vec3(0));

			}
		}
	}
}

void GameServer::_makeWorld() {
	EntityID mapID = world::invalidID;
	{
		ServerFreezePlayerPacket freeze{};
		freeze.action = ServerFreezePlayerPacket::Action::freeze;
		_server->sendDataToAll((char*)&freeze, freeze.len);
	}

	{
		ServerInitializePacket si{};
		si.ti.pos = { (ROOM_GRID_SIZE / 2 + 0.5f) * ROOM_SIZE, 3, (ROOM_GRID_SIZE / 2 + 0.5f) * ROOM_SIZE };
		si.ti.scale = { 1, 1, 1 };
		si.ti.rot = glm::quat(1, 0, 0, 0);
		for (Player* player : _players) {
			si.entityid = player->entityid;
			_server->sendDataToClient((char*)&si, si.len, player->serverid);
		}
	}

	ServerDeleteEntityPacket dp{};
	if (_tileGeneration)
		mapID = _tileGeneration->mapentity->id;

	_tileGeneration.reset();
	for (auto c : Hydra::Component::NetworkSyncComponent::componentHandler->getActiveComponents()) {
		auto e = world::getEntity(c->entityID);
		if (e->dead)
			continue;
		e->dead = true;
		dp.id = c->entityID;
		_server->sendDataToAll((char*)&dp, dp.len);
	}
	for (auto c : Hydra::Component::AIComponent::componentHandler->getActiveComponents()) {
		auto e = world::getEntity(c->entityID);
		if (e->dead)
			continue;
		e->dead = true;
		dp.id = c->entityID;
		_server->sendDataToAll((char*)&dp, dp.len);
	}

	_networkEntities.erase(std::remove_if(_networkEntities.begin(), _networkEntities.end(), [](const auto& e) {	return world::getEntity(e)->dead; }), _networkEntities.end());
	_deadSystem.tick(0);
	size_t tries = 0;
	const size_t minRoomCount = 25;
	const size_t maxRoomCount = 31;
	while (true) {

		tries++;
		//_tileGeneration->level = level;
		if (level < 2) {
			_tileGeneration = std::make_unique<TileGeneration>(maxRoomCount, "assets/room/starterRoom.room", &GameServer::_onRobotShoot, static_cast<void*>(this), level);
			_spawnerSystem.userdata = static_cast<void*>(this);
			_spawnerSystem.onShoot = &GameServer::_onRobotShoot;
		}
		else {
			_tileGeneration = std::make_unique<TileGeneration>(1, "assets/BossRoom/Bossroom5.room", &GameServer::_onRobotShoot, static_cast<void*>(this), level);
			ServerFreezePlayerPacket freeze{};
			freeze.action = ServerFreezePlayerPacket::Action::noPVS;
			_server->sendDataToAll((char*)&freeze, freeze.len);
		}
		_tileGeneration->buildMap();
		if (level < 2) {
			_deadSystem.tick(0);
			printf("Room count: %zu\t(%zu)\n", Hydra::Component::RoomComponent::componentHandler->getActiveComponents().size(), _tileGeneration->roomCounter);
			if (Hydra::Component::RoomComponent::componentHandler->getActiveComponents().size() >= minRoomCount)
				break;
			printf("\tTarget is >= %zu, redoing generation\n", minRoomCount);
			_tileGeneration.reset();
			_deadSystem.tick(0);
		}
		else {
			_deadSystem.tick(0);
			_spawnBoss();
			break;
		}
	}
	printf("\tTook %zu tries\n", tries);
	_tileGeneration->spawnDoors();
	_tileGeneration->spawnEnemies();
	_tileGeneration->createSpawner();
	_tileGeneration->spawnPickUps();
	_tileGeneration->finalize();
	_pathfindingMap = _tileGeneration->pathfindingMap;
	PathFinding::setRoomGrid(_tileGeneration->roomGrid);
	std::vector<std::shared_ptr<Hydra::World::Entity>> allSpawners;
	world::getEntitiesWithComponents<Hydra::Component::SpawnerComponent>(allSpawners);
	for (int_openmp_t i = 0; i < (int_openmp_t)allSpawners.size(); i++) {
		auto sp = allSpawners[i]->getComponent<Hydra::Component::SpawnerComponent>();
		sp->map = _tileGeneration->pathfindingMap;
	}

	{
		auto packet = createServerSpawnEntity(_tileGeneration->mapentity.get());
		_server->sendDataToAll((char*)packet, packet->len);
		delete[](char*)packet;
	}

	{
		std::string map = _tileGeneration->getPathMapAsString();
		ServerPathMapPacket* spm = (ServerPathMapPacket*)new char[sizeof(ServerPathMapPacket) + WORLD_MAP_SIZE*WORLD_MAP_SIZE];
		*spm = ServerPathMapPacket(WORLD_MAP_SIZE*WORLD_MAP_SIZE);

		for (int y = 0; y < WORLD_MAP_SIZE; y++)
			for (int x = 0; x < WORLD_MAP_SIZE; x++)
				spm->data[y * WORLD_MAP_SIZE + x] = _tileGeneration->pathfindingMap[x][y];
		_server->sendDataToAll((char*)spm, spm->len);
		delete[](char*)spm;
	}

	SDL_Surface* map = SDL_CreateRGBSurface(0, WORLD_MAP_SIZE, WORLD_MAP_SIZE, 32, 0, 0, 0, 0);
	{
		auto color = colors[Tile::Void];
		SDL_FillRect(map, nullptr, SDL_MapRGB(map->format, color.x, color.y, color.z));
	}

	for (size_t x = 0; x < ROOM_GRID_SIZE; x++)
		printf("|   %zu  ", x);
	printf("|\n");

	for (int y = 0; y < ROOM_GRID_SIZE; y++) {
		for (int x = 0; x < ROOM_GRID_SIZE; x++) {
			std::shared_ptr<Hydra::Component::RoomComponent> rc = _tileGeneration->roomGrid[x][y];
			if (rc) {
				printf("| %c%c%c%c ",
					rc->door[Hydra::Component::RoomComponent::NORTH] ? 'N' : ' ',
					rc->door[Hydra::Component::RoomComponent::EAST] ? 'E' : ' ',
					rc->door[Hydra::Component::RoomComponent::SOUTH] ? 'S' : ' ',
					rc->door[Hydra::Component::RoomComponent::WEST] ? 'W' : ' '
				);

				// Walls
				auto pfm = _pathfindingMap;
				auto drawWallPixel = [pfm, map](int x, int y) {
					SDL_Rect rect{ x, y, 1, 1 };
					auto color = colors[Tile::Wall];
					if (pfm[x][y] && x > 0 && y > 0 && x < WORLD_MAP_SIZE - 1 && y < WORLD_MAP_SIZE - 1)
						color = colors[Tile::Portal];
					SDL_FillRect(map, &rect, SDL_MapRGB(map->format, color.x, color.y, color.z));
				};
				auto drawRoomPixel = [pfm, map](int x, int y) {
					SDL_Rect rect{ x, y, 1, 1 };
					auto color = colors[Tile::Air];
					if (pfm[x][y] && x > 0 && y > 0 && x < WORLD_MAP_SIZE - 1 && y < WORLD_MAP_SIZE - 1)
						color = colors[Tile::RoomContent];
					SDL_FillRect(map, &rect, SDL_MapRGB(map->format, color.x, color.y, color.z));
				};

				for (int i = 0; i < ROOM_MAP_SIZE; i++)
					drawWallPixel(x * ROOM_MAP_SIZE + i, y * ROOM_MAP_SIZE);

				for (int i = 0; i < ROOM_MAP_SIZE; i++)
					drawWallPixel(x * ROOM_MAP_SIZE + i, (y + 1) * ROOM_MAP_SIZE - 1);

				for (int i = 0; i < ROOM_MAP_SIZE; i++)
					drawWallPixel(x * ROOM_MAP_SIZE, y * ROOM_MAP_SIZE + i);

				for (int i = 0; i < ROOM_MAP_SIZE; i++)
					drawWallPixel((x + 1) * ROOM_MAP_SIZE - 1, y * ROOM_MAP_SIZE + i);

				// Center of room
				SDL_Rect rect = { x * ROOM_MAP_SIZE + 1, y * ROOM_MAP_SIZE + 1, ROOM_MAP_SIZE - 2, ROOM_MAP_SIZE - 2 };
				auto color = colors[Tile::Air];
				SDL_FillRect(map, &rect, SDL_MapRGB(map->format, color.x, color.y, color.z));

				for (int i = 1; i < ROOM_MAP_SIZE - 1; i++)
					for (int j = 1; j < ROOM_MAP_SIZE - 1; j++)
						drawRoomPixel(x * ROOM_MAP_SIZE + i, y * ROOM_MAP_SIZE + j);
			}
			else {
				printf("|      ");
			}
		}
		printf("|\n");
	}

	SDL_SaveBMP(map, "map.bmp");
	SDL_FreeSurface(map);
#ifdef _WIN32
	system("PVSTest.exe -f map.bmp -s 32 -o map.pvs");
#else
	system("bin/PVSTest -f map.bmp -s 32 -o map.pvs");
#endif

	{
		FILE* fp = fopen("map.pvs", "rb");
		fseek(fp, 0, SEEK_END);
		_pvsData.resize(ftell(fp));
		fseek(fp, 0, SEEK_SET);
		fread(_pvsData.data(), _pvsData.size(), 1, fp);
		fclose(fp);
}

	{
		std::vector<std::shared_ptr<Entity>> entities;
		world::getEntitiesWithComponents<NetworkSyncComponent>(entities);
		for (size_t i = 0; i < entities.size(); i++) {
			auto p = createServerSpawnEntity(entities[i].get());
			_server->sendDataToAll((char*)p, p->len);
			delete[](char*)p;
		}
	}

	{
		ServerInitializePVSPacket* pvs = (ServerInitializePVSPacket*)new char[sizeof(ServerInitializePVSPacket) + _pvsData.size()];
		*pvs = ServerInitializePVSPacket(_pvsData.size());
		memcpy(pvs->data, _pvsData.data(), _pvsData.size());
		_server->sendDataToAll((char*)pvs, pvs->len);
		delete[](char*)pvs;
	}
	if (mapID != world::invalidID) {
		dp.id = mapID;
		_server->sendDataToAll((char*)&dp, dp.len);
	}

	for (auto& rb : Hydra::Component::RigidBodyComponent::componentHandler->getActiveComponents()) {
		//_engine->log(Hydra::LogLevel::normal, "Enabling BulletPhysicsSystem for %s", world::getEntity(rb->entityID)->name.c_str());
		_physicsSystem.enable(static_cast<Hydra::Component::RigidBodyComponent*>(rb.get()));
	}
	for (auto& goc : Hydra::Component::GhostObjectComponent::componentHandler->getActiveComponents()) {
		//_engine->log(Hydra::LogLevel::normal, "Enabling BulletPhysicsSystem for %s", world::getEntity(goc->entityID)->name.c_str());
		static_cast<Hydra::Component::GhostObjectComponent*>(goc.get())->updateWorldTransform();
		_physicsSystem.enable(static_cast<Hydra::Component::GhostObjectComponent*>(goc.get()));
	}

	std::vector<std::shared_ptr<Entity>> entities;
	world::getEntitiesWithComponents<NetworkSyncComponent>(entities);
	for (size_t i = 0; i < entities.size(); i++)
		_networkEntities.push_back(entities[i]->id);

	{
		ServerFreezePlayerPacket freeze{};
		freeze.action = ServerFreezePlayerPacket::Action::unfreeze;
		_server->sendDataToAll((char*)&freeze, freeze.len);
	}
}

void GameServer::run() {
	//Check for new Clients
	auto nowTime = std::chrono::high_resolution_clock::now();
	float delta = std::chrono::duration<float, std::chrono::milliseconds::period>(nowTime - _lastTime).count() / 1000.f;
	_lastTime = nowTime;
	{
		this->_handleDisconnects();
		this->_addPlayer(this->_server->checkForNewClients());
	}

	//Incoming packets
	{
		this->_resolvePackets(this->_server->receiveData());
	}

	//Update World
	{
		static std::vector<std::shared_ptr<Entity>> ents;
		world::getEntitiesWithComponents<Hydra::Component::AIComponent>(ents);
		for (size_t k = 0; k < ents.size(); k++) {
			TransformComponent* ptc = ents[k]->getComponent<TransformComponent>().get();
			float distance = FLT_MAX;
			int target = -1;
			for (size_t i = 0; i < this->_players.size(); i++) {
				TransformComponent* tc = world::getEntity(this->_players[i]->entityid)->getComponent<TransformComponent>().get();
				float f = glm::distance(ptc->position, tc->position);
				if (f < distance) {
					target = i;
					distance = f;
				}
			}
			auto ai = ents[k]->getComponent<AIComponent>().get();
			if (target != -1) {
				ai->behaviour->setTargetPlayer(world::getEntity(this->_players[target]->entityid));
			}
		}
		ents.clear();

		static std::vector<std::shared_ptr<Entity>> spawnEnts;
		world::getEntitiesWithComponents<Hydra::Component::SpawnerComponent>(spawnEnts);
		for (size_t k = 0; k < spawnEnts.size(); k++) {
			TransformComponent* ptc = spawnEnts[k]->getComponent<TransformComponent>().get();
			float distance = FLT_MAX;
			int target = -1;
			for (size_t i = 0; i < this->_players.size(); i++) {
				TransformComponent* tc = world::getEntity(this->_players[i]->entityid)->getComponent<TransformComponent>().get();
				float f = glm::distance(ptc->position, tc->position);
				if (f < distance) {
					target = i;
					distance = f;
				}
			}
			auto spawn = spawnEnts[k]->getComponent<Hydra::Component::SpawnerComponent>().get();
			if (target != -1) {
				spawn->setTargetPlayer(world::getEntity(this->_players[target]->entityid));
			}
		}
		spawnEnts.clear();

		_deadSystem.tick(delta);
		_physicsSystem.tick(delta);
		_aiSystem.tick(delta);
		_bulletSystem.tick(delta);
		////_abilitySystem.tick(delta);
		_spawnerSystem.tick(delta);
		{
			for (size_t i = 0; i < _spawnerSystem.didJustSpawn.size(); i++) {
				auto e = _spawnerSystem.didJustSpawn[i];
				_networkEntities.push_back(e->id);
				auto p = createServerSpawnEntity(e);
				_server->sendDataToAll((char*)p, p->len);
				delete[](char*)p;
			}
		}

		//_perkSystem.tick(delta);
		_lifeSystem.tick(delta);
		_pickupSystem.tick(delta);

		//�NNU MER FUSK KOD JAAAAAA
		//std::vector<std::shared_ptr<Entity>> children;
		//world::getEntitiesWithComponents<PickUpComponent>(children);
		//for (size_t i = 0; i < children.size(); i++) {
		//	auto putc = children[i]->getComponent<TransformComponent>();
		//	for (size_t j = 0; j < _players.size(); j++) {
		//		auto ptc = world::getEntity(_players[j]->entityid)->getComponent<TransformComponent>();
		//		if (glm::distance(putc->position, ptc->position) < 10.f) {
		//			ServerDeleteEntityPacket p{};
		//			p.id = children[i]->id;
		//			_server->sendDataToAllExcept((char*)&p, p.len, _players[j]->serverid);
		//
		//			_networkEntities.erase(std::remove_if(_networkEntities.begin(), _networkEntities.end(), [p](const auto& e) { return e == p.id; }), _networkEntities.end());
		//			children[i]->dead = true;
		//			continue;
		//		}
		//	}
		//}

		//END

		for (Hydra::World::EntityID eID : _lifeSystem.isKilled()) {
			if (auto e = world::getEntity(eID)) {
				if (auto ai = e->getComponent<Hydra::Component::AIComponent>(); ai) {
					auto oldTransform = e->getComponent<Hydra::Component::TransformComponent>();
					auto oldMesh = e->getComponent<Hydra::Component::MeshComponent>();
					if (!oldTransform || !oldMesh)
						continue;

					char name[64] = { 0 };
					snprintf(name, sizeof(name), "Dead body [%zu]", eID);
					auto deadBody = world::newEntity(name, e->parent);
					auto t = deadBody->addComponent<Hydra::Component::TransformComponent>();
					t->position = oldTransform->position;
					t->scale = oldTransform->scale;
					t->rotation = oldTransform->rotation;
					t->dirty = true;
					auto mesh = deadBody->addComponent<Hydra::Component::MeshComponent>();
					mesh->loadMesh(oldMesh->meshFile);
					auto life = deadBody->addComponent<Hydra::Component::LifeComponent>();
					life->health = life->maxHP = 25.0f / 24.0f;
					life->tickDownWithTime = true;

					switch (ai->behaviour->type) {
					case Behaviour::Type::ALIEN:
						mesh->currentFrame = 1;
						mesh->animationIndex = 3;
						break;

					case Behaviour::Type::ROBOT:
						mesh->currentFrame = 1;
						mesh->animationIndex = 3;
						break;

					case Behaviour::Type::BOSS_HAND:
					case Behaviour::Type::BOSS_ARMS:
					case Behaviour::Type::STATINARY_BOSS:
						// TODO:
						break;
					}

					printf("Syncing (%zu): %s\n", deadBody->id, deadBody->name.c_str());
					auto p = createServerSpawnEntity(deadBody.get());
					_server->sendDataToAll((char*)p, p->len);
					delete[](char*)p;
					deadBody->dead = true;
				}

				deleteEntity(eID);

				_players.erase(std::remove_if(_players.begin(), _players.end(), [eID](const auto& p) { return p->entityid == eID; }), _players.end());
			}
		}
		if (!Hydra::Component::AIComponent::componentHandler->getActiveComponents().size() || (level == 2 && !world::getEntity(_bossID))) {
			level++;
			if (level > 2) {
				ServerFreezePlayerPacket freeze{};
				freeze.action = ServerFreezePlayerPacket::Action::win;
				_server->sendDataToAll((char*)&freeze, freeze.len);

				level = 0;
				_makeWorld();

				printf("\n\n\n\n");
				printf("================SERVER INFO================\n");
				printf("The boss has been defeated, resetting server\n");
				printf("===========================================\n");
			} else
				_makeWorld();
		}
	}

	for (Player* p : _players) {
#undef max
		p->shootAnimation = std::max(0.0f, p->shootAnimation - delta);
	}

	//Send updated world to clients
	{
		_packetDelay += delta;
		/*if (_packetDelay >= 1.0f/30.0f)*/ {
			this->_sendWorld();
			_packetDelay = 0;
		}
		mySleep(1000 / 30);
	}
	_sendPathInfo();
}

void GameServer::quit() {
	for (size_t i = 0; i < this->_players.size(); i++)
		delete this->_players[i];
	delete this->_server;
}

void GameServer::syncEntity(Hydra::World::Entity* entity) {
	printf("Syncing (%zu): %s\n", entity->id, entity->name.c_str());

	entity->addComponent<Hydra::Component::TransformComponent>();
	this->_networkEntities.push_back(entity->id);

	auto p = createServerSpawnEntity(entity);
	_server->sendDataToAll((char*)p, p->len);
	delete[](char*)p;
}

void GameServer::_sendWorld() {
	ServerUpdatePacket* packet = (ServerUpdatePacket*)new char[(sizeof(ServerUpdatePacket) + (sizeof(ServerUpdatePacket::EntUpdate) * this->_networkEntities.size()))];
	*packet = ServerUpdatePacket(_networkEntities.size());
	for (size_t i = 0; i < this->_networkEntities.size(); i++) {
		ServerUpdatePacket::EntUpdate& entupdate = packet->data[i];
		Entity* entity = world::getEntity(this->_networkEntities[i]).get();
		if (entity) {
			this->_convertEntityToTransform(packet->data[i], this->_networkEntities[i]);

			auto life = entity->getComponent<LifeComponent>();
			if (life)
				entupdate.life = life->health;
			else
				entupdate.life = INT32_MAX;

			auto mesh = entity->getComponent<MeshComponent>();
			if (mesh)
				entupdate.animationIndex = mesh->animationIndex;
			else
				entupdate.animationIndex = 0;
		} else
			this->_networkEntities.erase(_networkEntities.begin() + i);
	}

	this->_server->sendDataToAll((char*)packet, packet->len);
	delete[] packet;
}

void GameServer::_convertEntityToTransform(ServerUpdatePacket::EntUpdate& dest, EntityID ent) {
	Hydra::Component::TransformComponent* tc = World::getEntity(ent)->getComponent<Hydra::Component::TransformComponent>().get();
	dest.entityid = ent;
	dest.ti.pos = tc->position;
	dest.ti.scale = tc->scale;
	dest.ti.rot = tc->rotation;
}

void GameServer::_resolvePackets(std::vector<Hydra::Network::Packet*> packets) {
	for (size_t i = 0; i < packets.size(); i++) {
		auto& p = packets[i];
		auto entity = this->_getEntityID(p->client);
		if (!entity)
			continue;
		auto player = this->_getPlayer(entity);
		if (!player)
			continue;
		//printf("Resolving:\n\ttype: %s\n\tlen: %d\n", PacketTypeName[p->type], p->len);
		switch (p->type) {
		case PacketType::ClientUpdate:
			resolveClientUpdatePacket(player, (ClientUpdatePacket*)p, entity);
			break;

		case PacketType::ClientSpawnEntity:
			resolveClientSpawnEntityPacket((ClientSpawnEntityPacket*)p, entity, this->_server);
			break;

		case PacketType::ClientUpdateBullet:
			resolveClientUpdateBulletPacket((ClientUpdateBulletPacket*)p, player->bullet); // SUPER INEFFICIENT
			createAndSendPlayerUpdateBulletPacket(player, this->_server);
			break;

		case PacketType::ClientShoot:
			resolveClientShootPacket((ClientShootPacket*)p, player, &_physicsSystem); //SUPER INEFFICIENT
			createAndSendPlayerShootPacket(player, (ClientShootPacket*)p, this->_server);
			break;
		case PacketType::ClientRequestAIInfo:
			_resolveClientRequestAIInfoPacket((ClientRequestAIInfoPacket*)p);
			break;
		default:
			break;
		}
	}

	for (size_t i = 0; i < packets.size(); i++)
		delete packets[i];
}

int64_t GameServer::_getEntityID(int serverid) {
	for (size_t i = 0; i < this->_players.size(); i++)
		if (this->_players[i]->serverid == serverid)
			return this->_players[i]->entityid;
	return INT64_MAX;
}

void GameServer::_setEntityID(int serverID, int64_t entityID) {
	for (size_t i = 0; i < this->_players.size(); i++)
		if (this->_players[i]->serverid == serverID) {
			this->_players[i]->entityid = entityID;
			return;
		}
}

void GameServer::deleteEntity(EntityID ent) {
	ServerDeleteEntityPacket* sdp = createServerDeleteEntityPacket(ent);

	this->_server->sendDataToAll((char*)sdp, sdp->len);

	delete sdp;

	//optimize
	_networkEntities.erase(std::remove_if(_networkEntities.begin(), _networkEntities.end(), [ent](const auto& e) { return e == ent; }), _networkEntities.end());

	if (auto e = world::getEntity(ent); e)
		e->dead = true;
}

void GameServer::_handleDisconnects() {
	std::vector<int> vec = this->_server->getDisconnects();
	for (size_t i = 0; i < vec.size(); i++) {
		for (size_t k = 0; k < this->_players.size(); k++) {
			if (this->_players[k]->serverid == vec[i]) {
				ServerDeleteEntityPacket* sdp = createServerDeleteEntityPacket(this->_players[k]->entityid);
				this->_server->sendDataToAll((char*)sdp, sdp->len);
				printf("Player disconnected, entity id : %zu\n", this->_players[k]->entityid);
				for (size_t j = 0; j < this->_networkEntities.size(); j++) {
					if (this->_networkEntities[j] == this->_players[k]->entityid) {
						deleteEntity(this->_networkEntities[j]);
						break;
					}
				}
				this->_players.erase(this->_players.begin() + k);
				break;
			}
		}
	}
}

//NO PARENT IMPLEMENTATION YET (ALWAYS CREATES IN ROOT)
Entity* GameServer::_createEntity(const std::string& name, EntityID parentID, bool serverSynced) {
	Entity* ent = World::newEntity(name, parentID).get();

	printf("Created entity \"%s\" with entity id : %zu\n", ent->name.c_str(), ent->id);
	if (serverSynced) {
		ent->addComponent<Hydra::Component::TransformComponent>();
		this->_networkEntities.push_back(ent->id);

		auto p = createServerSpawnEntity(ent);
		_server->sendDataToAll((char*)p, p->len);
		delete[](char*)p;
		return ent;
	}
	else
		return ent;
}

Player* GameServer::_getPlayer(EntityID id) {
	for (size_t i = 0; i < this->_players.size(); i++)
		if (this->_players[i]->entityid == id)
			return this->_players[i];
	return nullptr;
}

bool GameServer::_addPlayer(int id) {
	if (id != -1) {
		Player* p;
		p = new Player();
		p->serverid = id;
		p->connected = true;
		this->_players.push_back(p);
		{
			ServerFreezePlayerPacket freeze{};
			freeze.action = ServerFreezePlayerPacket::Action::freeze;
			_server->sendDataToClient((char*)&freeze, freeze.len, id);
		}

		ServerInitializePacket pi{};
		Entity* enttmp = World::newEntity("Player", World::root()).get();
		enttmp->addComponent<PerkComponent>();
		{
			pi.entityid = enttmp->id;
			this->_setEntityID(id, pi.entityid);
			pi.ti.pos = { (ROOM_GRID_SIZE / 2 + 0.5f) * ROOM_SIZE, 3, (ROOM_GRID_SIZE / 2 + 0.5f) * ROOM_SIZE };
			pi.ti.scale = { 1, 1, 1 };
			pi.ti.rot = glm::quat(1, 0, 0, 0);

			auto rbc = enttmp->addComponent<Hydra::Component::RigidBodyComponent>();
			rbc->createBox(glm::vec3(1.0f, 2.0f, 1.0f), glm::vec3(0, 2, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER, 100,
				0, 0, 0.0f, 0);
			rbc->setAngularForce(glm::vec3(0, 0, 0));

			rbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableSimulation);

			if (_players.size() > 1) {
				auto randomOther = world::getEntity(_players[rand() % (_players.size() - 1)]->entityid);
				auto tc = randomOther->getComponent<TransformComponent>();
				pi.ti.pos = tc->position;
				pi.ti.rot = tc->rotation;
				pi.ti.scale = tc->scale;
			}

			auto mesh = enttmp->addComponent<Hydra::Component::MeshComponent>();
			mesh->loadMesh("assets/objects/characters/PlayerModel2.mATTIC");
			mesh->animationIndex = 1;

			Hydra::Component::TransformComponent* tc = enttmp->addComponent<Hydra::Component::TransformComponent>().get();
			auto life = enttmp->addComponent<Hydra::Component::LifeComponent>().get();
			life->maxHP = 100;
			life->health = 100;
			tc->setPosition(pi.ti.pos);
			tc->setScale(pi.ti.scale);
			tc->setRotation(pi.ti.rot);

			auto rgbc = enttmp->addComponent<Hydra::Component::RigidBodyComponent>();
			rgbc->createBox(glm::vec3(1.0f, 2.0f, 1.0f) * tc->scale, glm::vec3(0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER, 100,
				0, 0, 0.0f, 0);
			rgbc->setAngularForce(glm::vec3(0, 0, 0));
			rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
			_physicsSystem.enable(rgbc.get());
			printf("sendDataToClient:\n\ttype: ServerInitialize\n\tlen: %zu\n", pi.len);
			int tmp = this->_server->sendDataToClient((char*)&pi, pi.len, id);
		}

		{
			auto packet = createServerSpawnEntity(_tileGeneration->mapentity.get());
			_server->sendDataToClient((char*)packet, packet->len, id);
			delete[](char*)packet;
		}

		//TODO: Send all network sync entities _networkEntities?
		{
			std::vector<std::shared_ptr<Entity>> entities;
			world::getEntitiesWithComponents<NetworkSyncComponent>(entities);
			for (size_t i = 0; i < entities.size(); i++) {
				auto p = createServerSpawnEntity(entities[i].get());
				_server->sendDataToClient((char*)p, p->len, id);
				delete[](char*)p;
			}
		}

		{
			this->_networkEntities.push_back(p->entityid);
			printf("Player connected with entity id: %zu\n", pi.entityid);
		}


		if (level == 2) {
			ServerFreezePlayerPacket freeze{};
			freeze.action = ServerFreezePlayerPacket::Action::noPVS;
			_server->sendDataToClient((char*)&freeze, freeze.len, id);
		}

		{
			ServerInitializePVSPacket* pvs = (ServerInitializePVSPacket*)new char[sizeof(ServerInitializePVSPacket) + _pvsData.size()];
			*pvs = ServerInitializePVSPacket(_pvsData.size());
			memcpy(pvs->data, _pvsData.data(), _pvsData.size());
			_server->sendDataToClient((char*)pvs, pvs->len, id);
			delete[](char*)pvs;
		}

		{
			std::string map = _tileGeneration->getPathMapAsString();
			ServerPathMapPacket* spm = (ServerPathMapPacket*)new char[sizeof(ServerPathMapPacket) + WORLD_MAP_SIZE*WORLD_MAP_SIZE];
			*spm = ServerPathMapPacket(WORLD_MAP_SIZE*WORLD_MAP_SIZE);

			for (int y = 0; y < WORLD_MAP_SIZE; y++)
				for (int x = 0; x < WORLD_MAP_SIZE; x++)
					spm->data[y * WORLD_MAP_SIZE + x] = _tileGeneration->pathfindingMap[x][y];
			_server->sendDataToClient((char*)spm, spm->len, id);
			delete[](char*)spm;
		}

		//SEND A PACKET TO ALL OTHER CLIENTS
		ServerPlayerPacket* sppacket;
		for (size_t i = 0; i < this->_players.size(); i++) {
			if (this->_players[i]->entityid != p->entityid) {
				sppacket = createServerPlayerPacket("Knas", pi.ti);
				sppacket->entID = this->_players[i]->entityid;
				this->_server->sendDataToClient((char*)sppacket, sppacket->len, p->serverid);
				delete sppacket;
			}
		}

		ServerPlayerPacket* spp = createServerPlayerPacket("Fjant", pi.ti);
		spp->entID = p->entityid;
		printf("sendDataToAllExcept:\n\ttype: SERVERPLAYERPACKET\n\tlen: %zu\n", spp->len);
		this->_server->sendDataToAllExcept((char*)spp, spp->len, p->serverid);
		delete[](char*)spp;

		{
			ServerFreezePlayerPacket freeze{};
			freeze.action = ServerFreezePlayerPacket::Action::unfreeze;
			_server->sendDataToClient((char*)&freeze, freeze.len, id);
		}

		return true;
	}
	return false;
}

void BarcodeServer::GameServer::_sendPathInfo()
{
	for (auto clientAI : aiInspectorSync)
	{
		if (auto e = world::getEntity(clientAI.ai))
		{
			if (auto a = e->getComponent<Hydra::Component::AIComponent>())
			{
				if (a->behaviour->doDiddeliDoneDatPathfinding)
				{
					size_t open = a->behaviour->pathFinding->openList.size();
					size_t closed = a->behaviour->pathFinding->visitedList.size();
					size_t pathToEnd = a->behaviour->pathFinding->pathToEnd.size();
					//std::cout << "Vec2s sent:" << open << " " << closed << " " << pathToEnd << std::endl;
					if (open + closed + pathToEnd <= 0)
					{
						continue;
					}
					ServerAIInfoPacket* packet = (ServerAIInfoPacket*)new char[sizeof(ServerAIInfoPacket) + ((open + closed + pathToEnd) * 2 * sizeof(int))];
					*packet = ServerAIInfoPacket((open + closed + pathToEnd) * 2);

					packet->openList = open;
					packet->closedList = closed;
					packet->pathToEnd = pathToEnd;

					int i = 0;
					for (size_t o = 0; o < a->behaviour->pathFinding->openList.size(); o++)
					{
						packet->data[i] = a->behaviour->pathFinding->openList[o]->pos.x();
						i++;
						packet->data[i] = a->behaviour->pathFinding->openList[o]->pos.z();
						i++;
					}
					for (size_t c = 0; c < a->behaviour->pathFinding->visitedList.size(); c++)
					{
						packet->data[i] = a->behaviour->pathFinding->visitedList[c]->pos.x();
						i++;
						packet->data[i] = a->behaviour->pathFinding->visitedList[c]->pos.z();
						i++;
					}
					for (size_t p = 0; p < a->behaviour->pathFinding->pathToEnd.size(); p++)
					{
						glm::ivec2 pos = PathFinding::worldToMapCoords(a->behaviour->pathFinding->pathToEnd[p]).baseVec;
						packet->data[i] = pos.x;
						i++;
						packet->data[i] = pos.y;
						i++;
					}

					_server->sendDataToClient((char*)packet, packet->len, clientAI.client);
					a->behaviour->doDiddeliDoneDatPathfinding = false;
					delete[](char*)packet;
				}
			}
		}
	}
}

void GameServer::_onRobotShoot(WeaponComponent& weapon, Entity* bullet, void* userdata) {
	GameServer* this_ = static_cast<GameServer*>(userdata);

	{
		nlohmann::json json;
		bullet->serialize(json);
		std::vector<uint8_t> vec = nlohmann::json::to_msgpack(json);
		ServerUpdateBulletPacket* packet = (ServerUpdateBulletPacket*)new char[sizeof(ServerUpdateBulletPacket) + vec.size()];
		*packet = ServerUpdateBulletPacket(vec.size());
		packet->serverPlayerID = bullet->id; //TODO: FIX!
		memcpy(packet->data, vec.data(), vec.size());
		this_->_server->sendDataToAll((char*)packet, packet->len);
		delete[] packet;
	}

	{
		ServerShootPacket* packet = new ServerShootPacket();
		auto tc = bullet->getComponent<Hydra::Component::TransformComponent>().get();
		auto direction = bullet->getComponent<Hydra::Component::BulletComponent>()->direction;

		packet->direction = direction;
		packet->ti.pos = tc->position;
		packet->ti.scale = tc->scale;
		packet->ti.rot = tc->rotation;
		packet->serverPlayerID = bullet->id;

		this_->_server->sendDataToAll((char*)packet, sizeof(ServerShootPacket));
		delete packet;
	}
}

void BarcodeServer::GameServer::_resolveClientRequestAIInfoPacket(Hydra::Network::ClientRequestAIInfoPacket* packet)
{
	if (auto e = world::getEntity(packet->serverEntityID))
	{
		if (e->getComponent<Hydra::Component::AIComponent>())
		{
			for (size_t i = 0; i < aiInspectorSync.size(); i++)
			{
				if (aiInspectorSync[i].client == packet->client)
				{
					aiInspectorSync[i].ai = packet->serverEntityID;
					return;
				}
			}
			SyncBoi s;
			s.client = packet->client;
			s.ai = packet->serverEntityID;
			aiInspectorSync.push_back(s);
		}
	}
}
