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

void NetClient::_updateEntity(NetEntityInfo* psu) {
	if (psu->id == this->_player.getID()) {
		return;
	}

	//EntityID root = Hydra::World::World::getWorldRoot();
	//std::vector <EntityID> children = root->getChildren();
	//for (int i = 0; i < children.size(); i++) {
	//	if (children[i]->getID() == psu->id) {
	//		Hydra::Component::TransformComponent* tc = children[i]->getComponent<Hydra::Component::TransformComponent>();
	//		tc->setPosition(psu->ti.position);
	//		tc->setScale(psu->ti.scale);
	//		tc->setRotation(psu->ti.rot);
	//		break;
	//	}
	//}
}

void NetClient::update() {
	this->decodeNewPackets();
	if (this->_player.getID() > 0) {
		this->sendClientUpdatePacket();
	}
}

int NetClient::decodeNewPackets() {
	std::vector<NetPacket*> np = this->_tcp.receivePacket();
	std::vector<EntityID>* ents;
	Hydra::Component::TransformComponent* tc;
	std::shared_ptr<Entity> ent;
	//nlohmann::json json;
	for (int i = 0; i < np.size(); i++) {
		switch (np[i]->header.type) {
			//CREATE THE PLAYER!!!!!!!!!!!!!!!!!!!!!!!
		case PacketType::HelloWorld:
			this->_player.setID(((PacketHelloWorld*)np[i])->yourID);
			//this->_player.setPlayer(ent);
			//this->_player.setIsDead(false);
			//ent = world->createEntity("");
			ents = &Hydra::World::World::root()->children;
			for (int k = 0; k < ents->size(); k++) {
				Entity* ptr = Hydra::World::World::getEntity(ents->at(i)).get();
				if (ptr->name == "Player") {
					//ptr->setID(((PacketHelloWorld*)np[i])->yourID); FIX
					this->_player.addPlayer(ents->at(i));
				}
			}
			//ent->addComponent<Hydra::Component::TransformComponent>();
			//ent->addComponent<Hydra::Component::MeshComponent>("assets/objects/model1.ATTIC");
			break;
		case PacketType::ChangeID:
			break;
		case PacketType::ClientUpdate:
			break;
		case PacketType::SpawnEntityClient:
			break;
		case PacketType::SpawnEntityServer:
			ent = Hydra::World::World::newEntity("ERROR: SHIT PROBABLY FAILED", Hydra::World::World::root());
			{
				auto j = nlohmann::json::parse((char*)((PacketSpawnEntityServer*)np[i])->data);
				ent->deserialize(j);
			}
			//ent->setID(((PacketSpawnEntityServer*)np[i])->id); FIX
			//ent->addComponent<Hydra::Component::TransformComponent>();
			//tc = ent->getComponent<Hydra::Component::TransformComponent>();
			//tc->setPosition(((PacketSpawnEntityServer*)np[i])->ti.position);
			//tc->setRotation(((PacketSpawnEntityServer*)np[i])->ti.rot);
			//tc->setScale(((PacketSpawnEntityServer*)np[i])->ti.scale);
			//ent->addComponent<Hydra::Component::MeshComponent>("assets/objects/model1.ATTIC");
			break;
		case PacketType::ServerUpdate:
			for (int j = 0; j < ((PacketServerUpdate*)np[i])->nrOfEntity; j++) {
				this->_updateEntity(&((NetEntityInfo*)(((char*)np[i] + sizeof(PacketServerUpdate))))[j]);
			}
			this->_hasSentUpdate = false;
			//((NetEntityInfo*)(char*)(packet + sizeof(PacketServerUpdate)))
			break;
		}
	}

	return 0;
}

void NetClient::sendClientUpdatePacket() {
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
