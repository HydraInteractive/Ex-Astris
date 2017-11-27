#include <server/gameserver.hpp>
#include <server/packets.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <server/tilegeneration.hpp>
#include <iostream>
#include <chrono>
#include <thread>

using namespace BarcodeServer;
using namespace Hydra::Network;

using world = Hydra::World::World;

int64_t GameServer::_getEntityID(int serverid) {
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i]->serverid == serverid) {
			return this->_players[i]->entityid;
		}
	}
	return INT64_MAX;
}

void GameServer::_setEntityID(int serverID, int64_t entityID) {
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i]->serverid == serverID) {
			this->_players[i]->entityid = entityID;
			return;
		}
	}
}

void GameServer::_sendNewEntity(EntityID ent, int serverIDexception = -1) {
	//std::shared_ptr<Hydra::World::IEntity> ent = world->createEntity("CLIENT CREATED (ERROR)");
	nlohmann::json json;
	std::vector<uint8_t> data;
	World::getEntity(ent)->serialize(json);
	data = json.to_msgpack(json);


	ServerSpawnEntityPacket* ssep = new ServerSpawnEntityPacket();
	ssep->h.type = PacketType::ServerSpawnEntity;
	ssep->id = ent;
	ssep->size = data.size();
	ssep->h.len = ssep->getSize();
	if (serverIDexception == -1) {
		this->_server->sendDataToAll((char*)ssep, sizeof(ServerSpawnEntityPacket));
		this->_server->sendDataToAll((char*)data.data(), data.size() * sizeof(uint8_t));
	}
	else {
		this->_server->sendDataToAllExcept((char*)ssep, sizeof(ServerSpawnEntityPacket), serverIDexception);
		this->_server->sendDataToAllExcept((char*)data.data(), data.size() * sizeof(uint8_t), serverIDexception);
	}
	delete ssep;
}

void GameServer::_deleteEntity(EntityID ent) {

	ServerDeletePacket* sdp = createServerDeletePacket(ent);

	this->_server->sendDataToAll((char*)sdp, sdp->h.len);

	delete sdp;

	//optimize
	for (size_t i = 0; i < this->_networkEntities.size(); i++) {
		if (ent == this->_networkEntities[i]) {
			this->_networkEntities.erase(this->_networkEntities.begin() + i);
			break;
		}
	}
	world::getEntity(ent).get()->dead = true;
}

void GameServer::_handleDisconnects() {
	std::vector<int> vec = this->_server->getDisconnects();
	for (size_t i = 0; i < vec.size(); i++) {
		for (size_t k = 0; k < this->_players.size(); k++) {
			if (this->_players[k]->serverid == vec[i]) {
				ServerDeletePacket* sdp = createServerDeletePacket(this->_players[k]->entityid);
				this->_server->sendDataToAll((char*)sdp, sdp->h.len);
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
		_server->sendDataToAll((char*)p, p->h.len);
		delete[] (char*)p;
		return ent;
	}
	else
		return ent;
}

Player* GameServer::getPlayer(EntityID id) {
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i]->entityid == id) {
			return this->_players[i];
		}
	}
	return nullptr;
}


void GameServer::_convertEntityToTransform(ServerUpdatePacket::EntUpdate& dest, EntityID ent) {
	Hydra::Component::TransformComponent* tc = World::getEntity(ent)->getComponent<Hydra::Component::TransformComponent>().get();
	dest.entityid = ent;
	dest.ti.pos = tc->position;
	dest.ti.scale = tc->scale;
	dest.ti.rot = tc->rotation;
}

void GameServer::_sendWorld() {
	char* result = new char[(sizeof(ServerUpdatePacket) + (sizeof(ServerUpdatePacket::EntUpdate) * this->_networkEntities.size()))];
	ServerUpdatePacket* packet = (ServerUpdatePacket*)result;
	packet->h.type = PacketType::ServerUpdate;
	packet->h.len = (sizeof(ServerUpdatePacket) + (sizeof(ServerUpdatePacket::EntUpdate) * this->_networkEntities.size()));
	packet->nrOfEntUpdates = this->_networkEntities.size();
	for (size_t i = 0; i < this->_networkEntities.size(); i++) {
		this->_convertEntityToTransform(*(ServerUpdatePacket::EntUpdate*)((char*)result + sizeof(ServerUpdatePacket) + sizeof(ServerUpdatePacket::EntUpdate) * i), this->_networkEntities[i]);
	}

/*	if (_players.size())
		printf("sendDataToAll:\n\ttype: ServerUpdate\n\tlen: %d\n", packet->h.len);*/
	this->_server->sendDataToAll(result, packet->h.len);
	delete[] result;
}


void GameServer::_updateWorld() {
	
}

bool GameServer::_addPlayer(int id) {
	if (id != -1) {
		Player* p;
		p = new Player();
		p->serverid = id;
		p->connected = true;
		this->_players.push_back(p);
	
		//ADD COLLISION AND FUCK
		ServerInitializePacket pi;
		Entity* enttmp = World::newEntity("Player", World::root()).get();
		pi.entityid = enttmp->id;
		this->_setEntityID(id, pi.entityid);
		pi.h.len = sizeof(ServerInitializePacket);
		pi.h.type = PacketType::ServerInitialize;
		pi.ti.pos = { 0, 0, 0 };
		pi.ti.scale = { 1, 1, 1 };
		pi.ti.rot = glm::quat();
		
		printf("sendDataToClient:\n\ttype: ServerInitialize\n\tlen: %d\n", pi.h.len);
		int tmp = this->_server->sendDataToClient((char*)&pi, pi.h.len, id);
		


		//WORLD TEMP
		Entity* map = nullptr;
		std::vector<EntityID> children = world::root()->children;
		for (size_t i = 0; i < children.size(); i++) {
			map = world::getEntity(children[i]).get();
			if (map->name == "Map") {
				break;
			}
		}

		auto packet = createServerSpawnEntity(map);
		_server->sendDataToClient((char*)packet, packet->h.len, id);
		delete[] (char*)packet;

		//END TEMP WORLD











		Hydra::Component::TransformComponent* tc = enttmp->addComponent<Hydra::Component::TransformComponent>(/*pi.ti.pos, pi.ti.scale, pi.ti.rot*/).get();
		tc->setPosition(pi.ti.pos);
		tc->setScale(pi.ti.scale);
		tc->setRotation(pi.ti.rot);
		this->_networkEntities.push_back(p->entityid);
		printf("Player connected with entity id: %zu\n", pi.entityid);


		//SEND A PACKET TO ALL OTHER CLIENTS
		ServerPlayerPacket* sppacket;
		for (size_t i = 0; i < this->_players.size(); i++) {
			if (this->_players[i]->entityid != p->entityid) {
				sppacket = createServerPlayerPacket("Knas", pi.ti);
				sppacket->entID = this->_players[i]->entityid;
				this->_server->sendDataToClient((char*)sppacket, sppacket->getSize(), p->serverid);
				delete sppacket;
			}
		}

		
		ServerPlayerPacket* spp = createServerPlayerPacket("Fjant", pi.ti);
		spp->entID = p->entityid;
		printf("sendDataToAllExcept:\n\ttype: SERVERPLAYERPACKET\n\tlen: %d\n", spp->h.len);
		this->_server->sendDataToAllExcept((char*)spp, spp->getSize(), p->serverid);
		delete[] (char*)spp;
		
		return true;
	}
	return false;
}

void GameServer::_resolvePackets(std::vector<Hydra::Network::Packet*> packets) {
	for (size_t i = 0; i < packets.size(); i++) {
		auto& p = packets[i];
		//printf("Resolving:\n\ttype: %s\n\tlen: %d\n", PacketTypeName[p->h.type], p->h.len);
		switch (p->h.type) {
		case PacketType::ClientUpdate:
			resolveClientUpdatePacket((ClientUpdatePacket*)p, this->_getEntityID(p->h.client));
			break;

		case PacketType::ClientSpawnEntity:
			resolveClientSpawnEntityPacket((ClientSpawnEntityPacket*)p, this->_getEntityID(p->h.client), this->_server);
			break;

		case PacketType::ClientUpdateBullet:
			resolveClientUpdateBulletPacket((ClientUpdateBulletPacket*)p, this->getPlayer(this->_getEntityID(p->h.client))->bullet); // SUPER INEFFICIENT
			createAndSendPlayerUpdateBulletPacket(this->getPlayer(this->_getEntityID(p->h.client)), this->_server);
			break;

		case PacketType::ClientShoot:
			resolveClientShootPacket((ClientShootPacket*)p, this->getPlayer(this->_getEntityID(p->h.client)), &_physicsSystem); //SUPER INEFFICIENT
			createAndSendPlayerShootPacket(this->getPlayer(this->_getEntityID(p->h.client)), (ClientShootPacket*)p, this->_server);
			break;
		default:
			break;
		}
	}

	for (size_t i = 0; i < packets.size(); i++)
		delete packets[i];
}


GameServer::GameServer() {}

GameServer::~GameServer() {}

void GameServer::quit() {
	for (size_t i = 0; i < this->_players.size(); i++) {
		delete this->_players[i];
	}
	delete this->_server;
}

bool GameServer::initialize(int port) {
	if (!this->_server) {
		this->_server = new Server();
		this->lastTime = std::chrono::high_resolution_clock::now();
		packetDelay = 0;
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

/*static void markDead(EntityID id, bool dead) {
	std::shared_ptr<Entity> e = world::getEntity(id);
	if (!e)
		return;

	dead = e->dead = dead | e->dead;

	for (EntityID child : e->children)
		markDead(child, dead);
}

static void removeRelations(EntityID id) {
	std::shared_ptr<Entity> e = world::getEntity(id);
	if (!e)
		return;

	if (e->dead)
		e->parent = world::invalidID;

	for (EntityID child : e->children)
		removeRelations(child);

	std::remove_if(e->children.begin(), e->children.end(), [](EntityID c) { return world::getEntity(c)->dead; });
}*/

void GameServer::start() {
	TileGeneration* t = new TileGeneration("assets/room/starterRoom.room");
	t->buildMap();
	/* markDead(world::rootID, false);
	{
		FILE* fp = fopen("tree.dot", "w");
		fputs("digraph EntityTree {", fp);
		for (const std::shared_ptr<Entity>& e : world::_entities) {
			fprintf(fp, "E_%zu[label=\"%s\",color=%s,style=filled];", e->id, e->name.c_str(), e->dead ? "red" : "green");
			fprintf(fp, "E_%zu -> E_%zu[style=dotted];", e->id, e->parent);
			for (EntityID child : e->children)
				fprintf(fp, "E_%zu -> E_%zu;\n", e->id, child);
		}
		fputs("}", fp);
		fclose(fp);
	}
	removeRelations(world::rootID);*/
	_deadSystem.tick(0);

	printf("Room count: %zu\n", Hydra::Component::RoomComponent::componentHandler->getActiveComponents().size());
}

void GameServer::run() {
	//Check for new Clients
	auto nowTime = std::chrono::high_resolution_clock::now();
	float delta = std::chrono::duration<float, std::chrono::milliseconds::period>(nowTime - this->lastTime).count() / 1000.f;
	lastTime = nowTime;
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
		_deadSystem.tick(delta);
		_physicsSystem.tick(delta);
		_aiSystem.tick(delta);
		_bulletSystem.tick(delta);
		////_abilitySystem.tick(delta);
		_spawnerSystem.tick(delta);
		//_perkSystem.tick(delta);
		_lifeSystem.tick(delta);
		//this->_updateWorld();
	}

	//Send updated world to clients
	{
		this->packetDelay += delta;
		if (packetDelay >= 1.0f/30.0f) {
			this->_sendWorld();
			this->packetDelay = 0;
		}
	}
}

