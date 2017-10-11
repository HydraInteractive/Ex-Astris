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
HYDRA_API void NetClient::sendEntity(Hydra::World::IEntity * ent) {
	nlohmann::json json;
	ent->serialize(json);
	std::vector<uint8_t> vec = json.to_msgpack(json);
	ClientSpawnEntityPacket* packet = new ClientSpawnEntityPacket();
	packet->h.type = PacketType::ClientSpawnEntity;
	packet->size = vec.size();
	packet->h.len = packet->getSize();

	char* result = new char[sizeof(ClientSpawnEntityPacket) + vec.size() * sizeof(uint8_t)];
	
	memcpy(result, packet, sizeof(ClientSpawnEntityPacket));
	memcpy(result + sizeof(ClientSpawnEntityPacket), vec.data(), vec.size() * sizeof(uint8_t));

	this->_tcp.send(result, sizeof(ClientSpawnEntityPacket) + vec.size() * sizeof(uint8_t));
	//this->_tcp.send(packet, sizeof(ClientSpawnEntityPacket)); // DATA SNED
	//this->_tcp.send(vec.data(), vec.size() * sizeof(uint8_t)); // DATA SKJICJIK

	delete[] result;
	delete packet;
}

void NetClient::_resolvePackets(Hydra::World::IWorld* world) {
	std::vector<Packet*> packets = this->_tcp.receiveData();
	Hydra::Component::TransformComponent* tc;
	std::vector<std::shared_ptr<Hydra::World::IEntity>> children;
	std::shared_ptr<Hydra::World::IEntity> ent;
	Packet* serverUpdate = nullptr;
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
			//ent = world->createEntity("JagArEttSkott");
			//ent->addComponent<Hydra::Component::TransformComponent>();
			//this->_sendEntity(ent.get()); // KOLLAR SPAWN ENTITYPACKET OM DET FUNGERAR GUCCI
			break;

		case PacketType::ServerUpdate: 
			serverUpdate = packets[i];
			break;
		case PacketType::ServerPlayer:
			this->_addPlayer(world, packets[i]);
			break;
		case PacketType::ServerSpawnEntity:
			this->_resolveServerSpawnEntityPacket(world, (ServerSpawnEntityPacket*)packets[i]);
			break;
		case PacketType::ServerDeleteEntity:
			this->_resolveServerDeletePacket(world, (ServerDeletePacket*)packets[i]);
			break;
		}
	}

	if (serverUpdate)
		this->_updateWorld(world, serverUpdate);

	for (size_t i = 0; i < packets.size(); i++) {
		delete packets[i];
	}
}


HYDRA_API void NetClient::_resolveServerSpawnEntityPacket(Hydra::World::IWorld * world, ServerSpawnEntityPacket* entPacket) {
	nlohmann::json json;
	std::vector<uint8_t> vec;
	for (size_t i = 0; i < entPacket->size; i++) {
		vec.push_back(((uint8_t*)entPacket->data)[i]);
	}
	json = json.from_msgpack(vec);
	std::shared_ptr<Hydra::World::IEntity> ent = world->createEntity("SERVER CREATED (ERROR)");
	ent->deserialize(json);
	ent->setID(entPacket->id);
}

HYDRA_API void NetClient::_resolveServerDeletePacket(Hydra::World::IWorld* world, ServerDeletePacket* delPacket) {
	std::vector<std::shared_ptr<Hydra::World::IEntity>> children = world->getWorldRoot()->getChildren();
	for (size_t i = 0; i < children.size(); i++) {
		if (children[i]->getID() == delPacket->id) {
			children[i]->markDead();
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