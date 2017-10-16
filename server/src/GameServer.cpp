#include "GameServer.h"
#include "Packets.h"
#include <hydra/component/transformcomponent.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#define size_bajs size_t


int64_t GameServer::_getEntityID(int serverid) {
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i]->serverid == serverid) {
			return this->_players[i]->entityid;
		}
	}
}

void GameServer::_setEntityID(int serverID, int64_t entityID) {
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i]->serverid == serverID) {
			this->_players[i]->entityid = entityID;
			return;
		}
	}
}

void GameServer::_sendNewEntity(Hydra::World::IEntity* ent) {
	//std::shared_ptr<Hydra::World::IEntity> ent = world->createEntity("CLIENT CREATED (ERROR)");
	nlohmann::json json;
	std::vector<uint8_t> data;
	ent->serialize(json, true);
	data = json.to_msgpack(json);


	ServerSpawnEntityPacket* ssep = new ServerSpawnEntityPacket();
	ssep->h.type = PacketType::ServerSpawnEntity;
	ssep->id = ent->getID();
	ssep->size = data.size();
	ssep->h.len = ssep->getSize();

	this->_server->sendDataToAll((char*)ssep, sizeof(ServerSpawnEntityPacket));
	this->_server->sendDataToAll((char*)data.data(), data.size() * sizeof(uint8_t));

	delete ssep;
}

void GameServer::_deleteEntity(Hydra::World::IEntity * ent) {
	
	ServerDeletePacket* sdp = createServerDeletePacket(ent->getID());
	
	this->_server->sendDataToAll((char*)sdp, sdp->h.len);

	delete sdp;
	//optimize
	for (size_t i = 0; i < this->_networkEntities.size(); i++) {
		if (ent->getID() == this->_networkEntities[i]->getID()) {
			this->_networkEntities.erase(this->_networkEntities.begin() + i);
			break;
		}
	}
	ent->markDead();
}

void GameServer::_handleDisconnects() {
	std::vector<int> vec = this->_server->getDisconnects();
	for (size_t i = 0; i < vec.size(); i++) {
		for (size_t k = 0; k < this->_players.size(); k++) {
			if (this->_players[k]->serverid == vec[i]) {
				ServerDeletePacket* sdp = createServerDeletePacket(this->_players[k]->entityid);
				this->_server->sendDataToAll((char*)sdp, sdp->h.len);
				printf("Player disconnected, entity id : %d\n", this->_players[k]->entityid);
				for (size_t j = 0; j < this->_networkEntities.size(); j++) {
					if (this->_networkEntities[j]->getID() == this->_players[k]->entityid) {
						this->_networkEntities[j]->markDead();
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
Hydra::World::IEntity * GameServer::_createEntity(std::string name, int parentID, bool serverSynced) {
	std::shared_ptr<Hydra::World::IEntity> ent = this->_world->createEntity(name);

	printf("Created entity \"%s\" with entity id : %d\n", ent->getName().c_str(), ent->getID());
	if(serverSynced) {
		ent->addComponent<Hydra::Component::TransformComponent>();
		this->_networkEntities.push_back(ent);

		createAndSendServerEntityPacket(ent.get(), this->_server);
		return ent.get();
	}
	else
		return ent.get();
}


void GameServer::_convertEntityToTransform(ServerUpdatePacket::EntUpdate& dest, std::shared_ptr<Hydra::World::IEntity> ent) {
	Hydra::Component::TransformComponent* tc = ent->getComponent<Hydra::Component::TransformComponent>();
	dest.entityid = ent->getID();
	dest.ti.pos = tc->getPosition();
	dest.ti.scale = tc->getScale();
	dest.ti.rot = tc->getRotation();
}

void GameServer::_sendWorld() {
	char* result = new char[(sizeof(ServerUpdatePacket) + (sizeof(ServerUpdatePacket::EntUpdate) * this->_networkEntities.size()))];
	((ServerUpdatePacket*)result)->h.type = PacketType::ServerUpdate;
	((ServerUpdatePacket*)result)->h.len = (sizeof(ServerUpdatePacket) + (sizeof(ServerUpdatePacket::EntUpdate) * this->_networkEntities.size()));
	((ServerUpdatePacket*)result)->nrOfEntUpdates = this->_networkEntities.size();
	for (size_t i = 0; i < this->_networkEntities.size(); i++) {
		this->_convertEntityToTransform(*(ServerUpdatePacket::EntUpdate*)((char*)result + sizeof(ServerUpdatePacket) + sizeof(ServerUpdatePacket::EntUpdate) * i), this->_networkEntities[i]);
	}
	this->_server->sendDataToAll(result, ((ServerUpdatePacket*)result)->h.len);
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
		p->entityptr = this->_world->createEntity("Player");
		pi.entityid = p->entityptr->getID();
		this->_setEntityID(id, pi.entityid);
		pi.h.len = sizeof(ServerInitializePacket);
		pi.h.type = PacketType::ServerInitialize;
		pi.ti.pos = { 0, 0, 0 };
		pi.ti.scale = { 1, 1, 1 };
		pi.ti.rot = glm::quat();
		int tmp = this->_server->sendDataToClient((char*)&pi, pi.h.len, id);
		
		p->entityptr->addComponent<Hydra::Component::TransformComponent>(pi.ti.pos, pi.ti.scale, pi.ti.rot);
		this->_networkEntities.push_back(p->entityptr);
		printf("Player connected with entity id: %d\n", pi.entityid);
		ServerPlayerPacket* sppacket;
		for (size_t i = 0; i < this->_players.size(); i++) {
			if (this->_players[i]->entityid != p->entityid) {
				sppacket = createServerPlayerPacket("Knas", pi.ti);
				sppacket->entID = this->_players[i]->entityptr->getID();
				this->_server->sendDataToClient((char*)sppacket, sppacket->getSize(), p->serverid);
				delete sppacket;
			}
		}

		
		ServerPlayerPacket* spp = createServerPlayerPacket("Fjant", pi.ti);
		spp->entID = p->entityptr->getID();
		this->_server->sendDataToAllExcept((char*)spp, spp->getSize(), p->serverid);
		delete[] (char*)spp;
		
		return true;
	}
	return false;
}

void GameServer::_resolvePackets(std::vector<Packet*> packets) {
	for (size_t i = 0; i < packets.size(); i++) {
		switch (packets[i]->h.type) {
		case PacketType::ClientUpdate:
			resolveClientUpdatePacket(this->_world.get(), (ClientUpdatePacket*)packets[i], this->_getEntityID(packets[i]->h.client));
			break;
		case PacketType::ClientSpawnEntity:
			resolveClientSpawnEntityPacket(this->_world.get(), (ClientSpawnEntityPacket*)packets[i], this->_getEntityID(packets[i]->h.client), this->_server);
			break;
		}
	}

	for (size_t i = 0; i < packets.size(); i++) {
		delete packets[i];
	}
}


GameServer::GameServer() {
	this->_server = nullptr;
}

GameServer::~GameServer() {
}

void GameServer::quit() {
	for (size_t i = 0; i < this->_players.size(); i++) {
		delete this->_players[i];
	}
	delete this->_world.get();
	delete this->_server;
}

bool GameServer::initialize(int port) {
	if (!this->_server) {
		this->_server = new Server();
		this->lastTime = std::chrono::high_resolution_clock::now();
		packetDelay = 0;
		if (this->_server->initialize(port)) {
			this->_world = Hydra::World::World::create();
			printf("I am a scurb\n");
			printf("Server started.\n");

			return true;
		}
		delete this->_server;
	}
	return false;
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
		//for (size_t k = 0; k < 10000000; k++); //WORKING KAPPA
		this->_world->tick(TickAction::physics, delta);
	}

	//Send updated world to clients
	{
		this->packetDelay += delta;
		if (packetDelay >= 1/30) {
			this->_sendWorld();
			this->packetDelay = 0;
		}

		//if (packetDelay >= 0.01) {
		//	this->_sendWorld();
		//	this->_createEntity("DinMamma", 0, true);
		//	packetDelay -= 0.01;
		//}
	}
}

