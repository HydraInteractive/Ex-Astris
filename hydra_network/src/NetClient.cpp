#include <../hydra_network/include/NetClient.h>
#include <hydra\component\meshcomponent.hpp>

NetClient::NetClient() {
}

NetClient::~NetClient() {
}

bool NetClient::initialize(unsigned int port, char* ip) {
	this->_player.setID(-1);
	IPaddress tmp;
	tmp.port = port;
	return this->_tcp.initiate(tmp, ip);
}

void NetClient::_updateEntity(Hydra::World::IWorld* world, NetEntityInfo* psu) {
	if (psu->id == this->_player.getID()) {
		return;
	}

	std::shared_ptr<Hydra::World::IEntity> root = world->getWorldRoot();
	std::vector <std::shared_ptr<Hydra::World::IEntity>> children = root->getChildren();
	for (int i = 0; i < children.size(); i++) {
		if (children[i]->getID() == psu->id) {
			Hydra::Component::TransformComponent* tc = children[i]->getComponent<Hydra::Component::TransformComponent>();
			tc->setPosition(psu->ti.position);
			tc->setScale(psu->ti.scale);
			tc->setRotation(psu->ti.rot);
			break;
		}
	}
}

void NetClient::update(Hydra::World::IWorld* world) {
	this->decodeNewPackets(world);
	if (this->_player.getID() > 0) {
		this->sendClientUpdatePacket();
	}
}

int NetClient::decodeNewPackets(Hydra::World::IWorld* world) {
	std::vector<NetPacket*> np = this->_tcp.receivePacket();
	for (int i = 0; i < np.size(); i++) {
		std::shared_ptr<Hydra::World::IEntity> ent;
		switch (np[i]->header.type) {
			//CREATE THE PLAYER!!!!!!!!!!!!!!!!!!!!!!!
		case PacketType::HelloWorld:
			ent = world->createEntity("");
			ent->setID(((PacketHelloWorld*)np[i])->yourID);
			this->_player.setID(((PacketHelloWorld*)np[i])->yourID);
			//this->_player.setPlayer(ent);
			//this->_player.setIsDead(false);
			this->_player.addPlayer(ent);
			ent->addComponent<Hydra::Component::TransformComponent>();
			ent->addComponent<Hydra::Component::MeshComponent>("assets/objects/test.fbx");
			break;
		case PacketType::ChangeID:
			break;
		case PacketType::ClientUpdate:
			break;
		case PacketType::SpawnEntityClient:
			break;
		case PacketType::ServerUpdate:
			for (int j = 0; j < ((PacketServerUpdate*)np[i])->nrOfEntity; j++) {
				this->_updateEntity(world, &((NetEntityInfo*)(((char*)np[i] + sizeof(PacketServerUpdate))))[j]);
			}
			//((NetEntityInfo*)(char*)(packet + sizeof(PacketServerUpdate)))
			break;
		}
	}

	return 0;
}

HYDRA_API void NetClient::sendClientUpdatePacket() {
	PacketClientUpdate pcu;
	TransformInfo pi = this->_player.getPlayerInfo();
	pcu.header.type = PacketType::ClientUpdate;
	pcu.owner = this->_player.getID();
	pcu.position = pi.position;
	pcu.rotation = pi.rot;

	this->_tcp.sendPacket((char*)&pcu, sizeof(PacketClientUpdate));
}
