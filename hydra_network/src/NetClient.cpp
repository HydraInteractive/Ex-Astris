#include <NetClient.h>
#include <hydra/component/meshcomponent.hpp>

NetClient::NetClient() {
}

NetClient::~NetClient() {
}

bool NetClient::initialize(unsigned int port, const char* ip) {
	this->_hasSentUpdate = false;
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
	for (size_t i = 0; i < children.size(); i++) {
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
	for (size_t i = 0; i < np.size(); i++) {
		std::shared_ptr<Hydra::World::IEntity> ent;
		Hydra::Component::TransformComponent* tc;
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
			ent->addComponent<Hydra::Component::MeshComponent>("assets/objects/model1.ATTIC");
			break;
		case PacketType::ChangeID:
			break;
		case PacketType::ClientUpdate:
			break;
		case PacketType::SpawnEntityClient:
			break;
		case PacketType::SpawnEntityServer:
			ent = world->createEntity("Server Created");
			ent->setID(((PacketSpawnEntityServer*)np[i])->id);
			ent->addComponent<Hydra::Component::TransformComponent>();
			tc = ent->getComponent<Hydra::Component::TransformComponent>();
			tc->setPosition(((PacketSpawnEntityServer*)np[i])->ti.position);
			tc->setRotation(((PacketSpawnEntityServer*)np[i])->ti.rot);
			tc->setScale(((PacketSpawnEntityServer*)np[i])->ti.scale);
			ent->addComponent<Hydra::Component::MeshComponent>("assets/objects/model1.ATTIC");
			break;
		case PacketType::ServerUpdate:
			for (int64_t j = 0; j < ((PacketServerUpdate*)np[i])->nrOfEntity; j++) {
				this->_updateEntity(world, &((NetEntityInfo*)(((char*)np[i] + sizeof(PacketServerUpdate))))[j]);
			}
			this->_hasSentUpdate = false;
			//((NetEntityInfo*)(char*)(packet + sizeof(PacketServerUpdate)))
			break;
		}
	}

	return 0;
}

HYDRA_API void NetClient::sendClientUpdatePacket() {
	if (!this->_hasSentUpdate) {
		PacketClientUpdate pcu;
		TransformInfo pi = this->_player.getPlayerInfo();
		pcu.header.type = PacketType::ClientUpdate;
		pcu.owner = this->_player.getID();
		pcu.ti.position = pi.position;
		pcu.ti.rot = pi.rot;
		pcu.ti.scale = pi.scale;

		this->_tcp.sendPacket((char*)&pcu, sizeof(PacketClientUpdate));
		this->_hasSentUpdate = true;
	}
}
