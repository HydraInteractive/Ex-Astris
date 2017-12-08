#include <server/gameserver.hpp>
#include <server/packets.hpp>
#include <server/tilegeneration.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/networksynccomponent.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>

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
	Void,
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
	rgbcf->createStaticPlane(glm::vec3(0, 1, 0), 1, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_FLOOR
		, 0, 0, 0, 0.6f, 0);
	floor->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/Floor_v2.mATTIC");

	_makeWorld();
}
void GameServer::_makeWorld() {
	{
		ServerFreezePlayerPacket freeze;
		freeze.action = ServerFreezePlayerPacket::Action::freeze;
		_server->sendDataToAll((char*)&freeze, freeze.len);
	}
	_tileGeneration.reset();
	for (auto c : Hydra::Component::NetworkSyncComponent::componentHandler->getActiveComponents())
		world::getEntity(c->entityID)->dead = true;
	for (auto c : Hydra::Component::AIComponent::componentHandler->getActiveComponents())
		world::getEntity(c->entityID)->dead = true;

	_networkEntities.erase(std::remove_if(_networkEntities.begin(), _networkEntities.end(), [](const auto& e) { return world::getEntity(e)->dead; }), _networkEntities.end());
	_deadSystem.tick(0);
	size_t tries = 0;
	while (true) {
		tries++;
		_tileGeneration = std::make_unique<TileGeneration>("assets/room/starterRoom.room", &GameServer::_onRobotShoot, static_cast<void*>(this));
		_pathfindingMap = _tileGeneration->buildMap();
		_deadSystem.tick(0);
		printf("Room count: %zu\t(%zu)\n", Hydra::Component::RoomComponent::componentHandler->getActiveComponents().size(), _tileGeneration->roomCounter);
		if (Hydra::Component::RoomComponent::componentHandler->getActiveComponents().size() >= 32)
			break;
		printf("\tTarget is >= 32, redoing generation\n");
		_tileGeneration.reset();
		_deadSystem.tick(0);
	}
	printf("\tTook %zu tries\n", tries);

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
		ServerFreezePlayerPacket freeze;
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
		_deadSystem.tick(delta);
		_physicsSystem.tick(delta);
		_aiSystem.tick(delta);
		_bulletSystem.tick(delta);
		////_abilitySystem.tick(delta);
		_spawnerSystem.tick(delta);
		//_perkSystem.tick(delta);
		_lifeSystem.tick(delta);

		for (Hydra::World::EntityID e : _lifeSystem.isKilled()) {
			_deleteEntity(e);

			_players.erase(std::remove_if(_players.begin(), _players.end(), [e](const auto& p) { return p->entityid == e; }), _players.end());
		}

		if (!Hydra::Component::AIComponent::componentHandler->getActiveComponents().size()) {
			level++;
			if (level == 2) {
				ServerFreezePlayerPacket freeze;
				freeze.action = ServerFreezePlayerPacket::Action::win;
				_server->sendDataToAll((char*)&freeze, freeze.len);
			} else
				_makeWorld();
		}
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
}

void GameServer::quit() {
	for (size_t i = 0; i < this->_players.size(); i++)
		delete this->_players[i];
	delete this->_server;
}

void GameServer::syncEntity(Hydra::World::Entity* entity) {
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
			resolveClientUpdatePacket((ClientUpdatePacket*)p, entity);
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

void GameServer::_deleteEntity(EntityID ent) {
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
						_deleteEntity(this->_networkEntities[j]);
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
			ServerFreezePlayerPacket freeze;
			freeze.action = ServerFreezePlayerPacket::Action::freeze;
			_server->sendDataToClient((char*)&freeze, freeze.len, id);
		}

		ServerInitializePacket pi;
		Entity* enttmp = World::newEntity("Player", World::root()).get();
		{
			pi.entityid = enttmp->id;
			this->_setEntityID(id, pi.entityid);
			pi.ti.pos = { (ROOM_GRID_SIZE / 2 + 0.5f) * ROOM_SIZE, 3, (ROOM_GRID_SIZE / 2 + 0.5f) * ROOM_SIZE };
			pi.ti.scale = { 1, 1, 1 };
			pi.ti.rot = glm::quat(1, 0, 0, 0);

			if (_players.size() > 1) {
				auto randomOther = world::getEntity(_players[rand() % (_players.size() - 1)]->entityid);
				auto tc = randomOther->getComponent<TransformComponent>();
				pi.ti.pos = tc->position;
				pi.ti.rot = tc->rotation;
				pi.ti.scale = tc->scale;
			}

			printf("sendDataToClient:\n\ttype: ServerInitialize\n\tlen: %zu\n", pi.len);
			int tmp = this->_server->sendDataToClient((char*)&pi, pi.len, id);
		}

		{
			Entity* map = nullptr;
			std::vector<EntityID> children = world::root()->children;
			for (size_t i = 0; i < children.size(); i++) {
				map = world::getEntity(children[i]).get();
				if (map->name == "Map")
					break;
			}

			auto packet = createServerSpawnEntity(map);
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
			Hydra::Component::TransformComponent* tc = enttmp->addComponent<Hydra::Component::TransformComponent>().get();
			auto life = enttmp->addComponent<Hydra::Component::LifeComponent>().get();
			life->maxHP = 100;
			life->health = 100;
			tc->setPosition(pi.ti.pos);
			tc->setScale(pi.ti.scale);
			tc->setRotation(pi.ti.rot);
			this->_networkEntities.push_back(p->entityid);
			printf("Player connected with entity id: %zu\n", pi.entityid);
		}

		{
			ServerInitializePVSPacket* pvs = (ServerInitializePVSPacket*)new char[sizeof(ServerInitializePVSPacket) + _pvsData.size()];
			*pvs = ServerInitializePVSPacket(_pvsData.size());
			memcpy(pvs->data, _pvsData.data(), _pvsData.size());
			_server->sendDataToClient((char*)pvs, pvs->len, id);
			delete[](char*)pvs;
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
			ServerFreezePlayerPacket freeze;
			freeze.action = ServerFreezePlayerPacket::Action::unfreeze;
			_server->sendDataToClient((char*)&freeze, freeze.len, id);
		}

		return true;
	}
	return false;
}


void GameServer::_onRobotShoot(WeaponComponent& weapon, Entity* bullet, void* userdata) {
	GameServer* this_ = static_cast<GameServer*>(userdata);

	printf("Robot shoot");

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
