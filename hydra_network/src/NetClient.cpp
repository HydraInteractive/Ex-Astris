#include <NetClient.h>
#include <hydra/component/meshcomponent.hpp>

void NetClient::_sendUpdatePacket() {
	if (this->_np.entptr) {
		ClientUpdatePacket cpup;
		Hydra::Component::TransformComponent* tc = this->_np.entptr->getComponent<Hydra::Component::TransformComponent>();
		cpup.ti.pos = tc->getPosition();
		cpup.ti.scale = tc->getScale();
		cpup.ti.rot = tc->getRotation();
		cpup.h.type = PacketType::ClientUpdate;
		cpup.h.len = sizeof(ClientUpdatePacket);

		this->_tcp.send(&cpup, cpup.h.len);
	}
}
void NetClient::_resolvePackets(Hydra::World::IWorld* world) {
	std::vector<Packet*> packets = this->_tcp.receiveData();
	Hydra::Component::TransformComponent* tc;
	std::vector<std::shared_ptr<Hydra::World::IEntity>> children;
	for (size_t i = 0; i < packets.size(); i++) {
		switch (packets[i]->h.type) {
		case PacketType::ServerInitialize:
			children = world->getWorldRoot()->getChildren();
			for (size_t i = 0; i < children.size(); i++) {
				if (children[i]->getName() == "Player") {
					this->_np.entptr = children[i].get();
					break;
				}
			}
			this->_np.entID = ((ServerInitializePacket*)packets[i])->entityid;
			this->_np.entptr->setID(this->_np.entID);
			tc = this->_np.entptr->getComponent<Hydra::Component::TransformComponent>();
			tc->setPosition(((ServerInitializePacket*)packets[i])->ti.pos);
			tc->setRotation(((ServerInitializePacket*)packets[i])->ti.rot);
			tc->setScale(((ServerInitializePacket*)packets[i])->ti.scale);
			break;

		case PacketType::ServerUpdate: 
			this->_updateWorld(world, packets[i]);
			break;
		case PacketType::ServerPlayer:
			this->_addPlayer(world, packets[i]);
			break;
		}

		
	}
}

HYDRA_API void NetClient::_updateWorld(Hydra::World::IWorld * world, Packet * updatePacket) {
	std::vector<std::shared_ptr<Hydra::World::IEntity>> children = world->getWorldRoot()->getChildren();
	ServerUpdatePacket* sup = (ServerUpdatePacket*)updatePacket;
	Hydra::Component::TransformComponent* tc;
	for (size_t k = 0; k < sup->nrOfEntUpdates; k++) {
		if (sup->data[k].entityid == this->_np.entID)
			continue;
		for (size_t i = 0; i < children.size(); i++) {
			if (children[i]->getID() == ((ServerUpdatePacket::EntUpdate)sup->data[k]).entityid) {
				tc = children[i]->getComponent<Hydra::Component::TransformComponent>();
				tc->setPosition(((ServerUpdatePacket::EntUpdate)sup->data[k]).ti.pos);
				tc->setRotation(((ServerUpdatePacket::EntUpdate)sup->data[k]).ti.rot);
				tc->setScale(((ServerUpdatePacket::EntUpdate)sup->data[k]).ti.scale);
				break;
			}
		}
	}
}

HYDRA_API void NetClient::_addPlayer(Hydra::World::IWorld * world, Packet * playerPacket) {
	ServerPlayerPacket* spp = (ServerPlayerPacket*)playerPacket;
	char* c = new char[spp->nameLength + 1];
	memcpy(c, spp->name, spp->nameLength);
	c[spp->nameLength] = '\0';

	std::shared_ptr<Hydra::World::IEntity> ent = world->createEntity(c);
	ent->setID(spp->entID);
	Hydra::Component::TransformComponent* tc = ent->addComponent<Hydra::Component::TransformComponent>();
	ent->addComponent<Hydra::Component::MeshComponent>("assets/objects/Table.ATTIC");
	tc->setPosition(spp->ti.pos);
	tc->setRotation(spp->ti.rot);
	tc->setScale(spp->ti.scale);
	delete[] c;
}

bool NetClient::initialize(char* ip, int port) {
	SDLNet_Init();
    if(this->_tcp.initialize(ip, port)) {
        this->_np.entptr = nullptr;
        this->_np.entID = -1;
		return true;
    }
	return false;
}

void NetClient::run(Hydra::World::IWorld* world) {
    {//Receive packets
        this->_resolvePackets(world);
    }

    //SendUpdate packet
    {
        this->_sendUpdatePacket();
    }

    //Nåt
    {
        
    }
}