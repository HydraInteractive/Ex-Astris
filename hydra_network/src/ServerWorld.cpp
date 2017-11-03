#include <Server/ServerWorld.hpp>
#include <Server/TCPHost.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include "json.hpp"

using namespace Hydra;
using namespace Hydra::World;

ServerWorld::ServerWorld() {
}

ServerWorld::~ServerWorld() {
}

void ServerWorld::initialize() {
}

HYDRA_NETWORK_API void ServerWorld::sendPlayers(TCPHost * _conn, int64_t target) {
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
	std::vector<EntityID>* ents = &Hydra::World::World::root()->children;
	Hydra::Component::TransformComponent* tc;
	for (size_t i = 0; i < this->_players.size(); i++) {
		if (this->_players[i].getID() != target) {
			//pes.id = this->_players[i].getID();
			//for (size_t j = 0; j < ents->size(); j++) {	
			//	if (int64_t(ents->at(i)) == this->_players[i].getID()) {
			//		tc = Hydra::World::World::getEntity(ents->at(i))->getComponent<Hydra::Component::TransformComponent>().get();
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
	return 0;
}

void ServerWorld::addEntity() {
	Hydra::World::World::newEntity("Bäst i test", Hydra::World::World::root());
}

ServerPlayer& ServerWorld::addPlayer(const glm::vec3 & pos, const glm::quat & rot, TCPsocket pSocket) {
	Entity* ent = Hydra::World::World::newEntity("Best i test", Hydra::World::World::root()).get();
	auto tc = ent->addComponent<Component::TransformComponent>();
	tc->setPosition(pos);
	tc->setScale(glm::vec3{ 1, 1, 1 });
	tc->setRotation(rot);
	ent->addComponent<Component::CameraComponent>();
	auto mc = ent->addComponent<Component::MeshComponent>();
	mc->meshFile = "assets/objects/alphaGunModel.ATTIC";
	ServerPlayer p;
	p.initialize(int64_t(ent), pSocket, int64_t(ent));
	this->_players.push_back(p);
	return _players.back();
}

HYDRA_NETWORK_API void ServerWorld::updateEntityTransform(TransformInfo pi, int64_t id) {
	const std::vector<EntityID> children = Hydra::World::World::root()->children;
	for (size_t i = 0; i < children.size(); i++) {
		if (int64_t(children[i]) == id) {
			Hydra::Component::TransformComponent* tc = Hydra::World::World::getEntity(children[i])->getComponent<Hydra::Component::TransformComponent>().get();
			tc->setPosition(pi.position);
			tc->setRotation(pi.rot);
			tc->setScale(pi.scale);
			return;
		}
	}
}

// ADD CHECK TO NOT SEND ALL ENTITIES ALWAYS
void ServerWorld::sendCurrentWorld(TCPHost* conn) {
	std::shared_ptr<Hydra::World::Entity> root = Hydra::World::World::root();
	const std::vector<EntityID> children = root->children;
	Hydra::Component::TransformComponent* tc;

	std::vector<EntityID> entities;
	//FILL WITH ALL ENTITES THAT SHOULD BE SENT OVER NETWORK
	for (size_t i = 0; i < children.size(); i++) {
		entities.push_back(children[i]);
	}

	PacketServerUpdate* packet = (PacketServerUpdate*)malloc(sizeof(PacketServerUpdate) + sizeof(NetEntityInfo) * entities.size());
	packet->header.type = PacketType::ServerUpdate;
	packet->nrOfEntity = entities.size();

	NetEntityInfo* tmp = ((NetEntityInfo*)((char*)packet + sizeof(PacketServerUpdate)));
	for (size_t i = 0; i < entities.size(); i++) {
		std::shared_ptr<Entity> enty = Hydra::World::World::getEntity(children[i]);
		tc = enty->getComponent<Hydra::Component::TransformComponent>().get();
		tmp[i].id = int64_t(children[i]);
		tmp[i].ti.position = tc->position;
		tmp[i].ti.rot = tc->rotation;
		tmp[i].ti.scale = tc->scale;
	}

	conn->sendToAllClients((char*)packet, (packet->getPacketSize()));
}

HYDRA_NETWORK_API void ServerWorld::sendEntity(TCPHost * _conn, Hydra::World::Entity * ent, int64_t exception) {
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

HYDRA_NETWORK_API std::vector<ServerPlayer> ServerWorld::getPlayers() {
	return this->_players;
}


HYDRA_NETWORK_API void ServerWorld::fill(std::shared_ptr<Hydra::World::Entity> ent, PacketSpawnEntityServer*& pse) {
	nlohmann::json j;
	ent->serialize(j);
	std::string str = j.dump();
	pse = (PacketSpawnEntityServer*)malloc(sizeof(PacketSpawnEntity) + str.size());
	pse->len = str.size();
	memcpy(pse->data, str.c_str(), str.size());
}


