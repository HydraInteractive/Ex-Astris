#include <Server/ServerWorld.hpp>
#include <Server/TCPHost.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include "json.hpp"

using namespace Hydra;
using namespace Hydra::World;

ServerWorld::ServerWorld() {
	this->_world = nullptr;
}

ServerWorld::~ServerWorld() {
}

void ServerWorld::initialize() {
	this->_world = Hydra::World::World::create(true);
}

HYDRA_API void ServerWorld::sendPlayers(TCPHost * _conn, int64_t target) {
	TCPsocket tmp = nullptr;
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i].getID() == target) {
			tmp = this->_players[i].getSocket();
		}
	}
	if (tmp == nullptr)
		return;

	PacketSpawnEntityServer pes;
	pes.header.type = PacketType::SpawnEntityServer;
	std::vector<std::shared_ptr<Hydra::World::IEntity>> ents = this->_world->getWorldRoot()->getChildren();
	Hydra::Component::TransformComponent* tc;
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i].getID() != target) {
			//pes.id = this->_players[i].getID();
			//for (size_t j = 0; j < ents.size(); j++) {
			//	if (ents[i]->getID() == this->_players[i].getID()) {
			//		tc = ents[i]->getComponent<Hydra::Component::TransformComponent>();
			//		pes.ti.position = tc->getPosition();
			//		pes.ti.rot = tc->getRotation();
			//		pes.ti.scale = tc->getScale();
			//		break;
			//	}
			//}
			//_conn->sendToClient((char*)&pes, sizeof(PacketSpawnEntityServer), tmp);
		}
	}

}

int ServerWorld::getNewEntityID() {
	return this->_world->getFreeID();
}

void ServerWorld::addEntity() {
	this->_world->createEntity("Bäst i test");
}

ServerPlayer& ServerWorld::addPlayer(const glm::vec3 & pos, const glm::quat & rot, TCPsocket pSocket) {
	std::shared_ptr<Hydra::World::IEntity> ent = this->_world->createEntity("Best i test");
	ent->addComponent<Component::TransformComponent>(pos, glm::vec3{ 1, 1, 1 }, rot);
	ent->addComponent<Component::CameraComponent>();
	ent->addComponent<Component::PlayerComponent>();
	ServerPlayer p;
	p.initialize(ent->getID(), pSocket, ent);
	this->_players.push_back(p);
	return _players.back();
}

HYDRA_API void ServerWorld::updateEntityTransform(TransformInfo pi, int64_t id) {
	const std::vector<std::shared_ptr<Hydra::World::IEntity>> children = this->_world->getWorldRoot()->getChildren();
	for (size_t i = 0; i < children.size(); i++) {
		if (children[i]->getID() == id) {
			Hydra::Component::TransformComponent* tc = children[i]->getComponent<Hydra::Component::TransformComponent>();
			tc->setPosition(pi.position);
			tc->setRotation(pi.rot);
			tc->setScale(pi.scale);
			return;
		}
	}
}

// ADD CHECK TO NOT SEND ALL ENTITIES ALWAYS
void ServerWorld::sendCurrentWorld(TCPHost* conn) {
	std::shared_ptr<Hydra::World::IEntity> root = this->_world->getWorldRoot();
	const std::vector<std::shared_ptr<Hydra::World::IEntity>> children = root->getChildren();
	Hydra::Component::TransformComponent* tc;

	std::vector<std::shared_ptr<Hydra::World::IEntity>> entities;
	//FILL WITH ALL ENTITES THAT SHOULD BE SENT OVER NETWORK
	for (size_t i = 0; i < children.size(); i++) {
		entities.push_back(children[i]);
	}

	PacketServerUpdate* packet = (PacketServerUpdate*)malloc(sizeof(PacketServerUpdate) + sizeof(NetEntityInfo) * entities.size());
	packet->header.type = PacketType::ServerUpdate;
	packet->nrOfEntity = entities.size();

	NetEntityInfo* tmp = ((NetEntityInfo*)((char*)packet + sizeof(PacketServerUpdate)));
	for (size_t i = 0; i < entities.size(); i++) {
		tc = children[i]->getComponent<Hydra::Component::TransformComponent>();
		tmp[i].id = children[i]->getID();
		tmp[i].ti.position = tc->getPosition();
		tmp[i].ti.rot = tc->getRotation();
		tmp[i].ti.scale = tc->getScale();
	}

	conn->sendToAllClients((char*)packet, (packet->getPacketSize()));
}

HYDRA_API void ServerWorld::sendEntity(TCPHost * _conn, Hydra::World::IEntity * ent, int64_t exception) {
	nlohmann::json json;
	ent->serialize(json);
	std::string str = json.dump();
	PacketSpawnEntityServer* se = (PacketSpawnEntityServer*)malloc(sizeof(PacketSpawnEntityServer) + str.size());
	se->len = str.size();
	memcpy(se->data, str.c_str(), str.size());

	//sendPacket(se);
	if (exception != -1) {
		for (int i = 0; i < this->_players.size(); i++) {
			if (exception == this->_players[i].getID()) {
				_conn->sendToAllExceptOne((char*)se, sizeof(PacketSpawnEntityServer) + str.size(), this->_players[i].getSocket());
				break;
			}
		}
	}
	else {
		_conn->sendToAllClients((char*)se, sizeof(PacketSpawnEntityServer) + str.size());
	}
	//free(se);
	delete[] (char*)se;
}

HYDRA_API std::vector<ServerPlayer> ServerWorld::getPlayers() {
	return this->_players;
}


HYDRA_API void ServerWorld::fill(std::shared_ptr<Hydra::World::IEntity> ent, PacketSpawnEntityServer*& pse) {
	nlohmann::json j;
	ent->serialize(j);
	std::string str = j.dump();
	pse = (PacketSpawnEntityServer*)malloc(sizeof(PacketSpawnEntity) + str.size());
	pse->len = str.size();
	memcpy(pse->data, str.c_str(), str.size());
}


