#include <../hydra_network/include/Server/ServerWorld.hpp>
#include <../hydra_network/include/Server/TCPHost.hpp>
#include <hydra/component/transformcomponent.hpp>

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

int ServerWorld::getNewEntityID() {
	return this->_world->getFreeID();
}

void ServerWorld::addEntity() {
	this->_world->createEntity("Bäst i test");
}

int64_t ServerWorld::addPlayer(const glm::vec3 & pos, const glm::quat & rot) {
	std::shared_ptr<Hydra::World::IEntity> ent = this->_world->createEntity("Best i test");
	ent->addComponent<Component::TransformComponent>(pos, glm::vec3{ 1, 1, 1 }, rot);
	return ent->getID();
}

HYDRA_API void ServerWorld::updateEntityTransform(TransformInfo pi, int64_t id) {
	const std::vector<std::shared_ptr<Hydra::World::IEntity>> children = this->_world->getWorldRoot()->getChildren();
	for (int i = 0; i < children.size(); i++) {
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
	for (int i = 0; i < children.size(); i++) {
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


