#include <NetClient.h>
#include <hydra/component/meshcomponent.hpp>

void NetClient::_sendUpdatePacket() {
	Entity* tmp = World::getEntity(this->_myID).get();
	if (tmp) {
		ClientUpdatePacket cpup;
		Hydra::Component::TransformComponent* tc = tmp->getComponent<Hydra::Component::TransformComponent>().get();
		cpup.ti.pos = tc->position;
		cpup.ti.scale = tc->scale;
		cpup.ti.rot = tc->rotation;
		cpup.h.type = PacketType::ClientUpdate;
		cpup.h.len = sizeof(ClientUpdatePacket);

		this->_tcp.send(&cpup, cpup.h.len);
	}
}
void NetClient::sendEntity(EntityID ent) {
	nlohmann::json json;
	Entity* entptr = World::getEntity(ent).get();
	entptr->serialize(json);
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

void NetClient::_resolvePackets() {
	std::vector<Packet*> packets = this->_tcp.receiveData();
	Hydra::Component::TransformComponent* tc;
	std::vector<EntityID> children;
	Entity* ent;
	Packet* serverUpdate = nullptr;
	for (size_t i = 0; i < packets.size(); i++) {
		switch (packets[i]->h.type) {
		case PacketType::ServerInitialize:
			children = World::root()->children;
			for (size_t i = 0; i < children.size(); i++) {
				ent = World::getEntity(children[i]).get();
				if (ent->name == "Player") {
					this->_myID = children[i];
					break;
				}
			}
			this->_IDs[((ServerInitializePacket*)packets[i])->entityid] = this->_myID;
			tc = World::getEntity(this->_myID)->getComponent<Hydra::Component::TransformComponent>().get();
			tc->setPosition(((ServerInitializePacket*)packets[i])->ti.pos);
			tc->setRotation(((ServerInitializePacket*)packets[i])->ti.rot);
			tc->setScale(((ServerInitializePacket*)packets[i])->ti.scale);
			break;
		case PacketType::ServerUpdate: 
			serverUpdate = packets[i];
			break;
		case PacketType::ServerPlayer:
			this->_addPlayer(packets[i]);
			break;
		case PacketType::ServerSpawnEntity:
			this->_resolveServerSpawnEntityPacket((ServerSpawnEntityPacket*)packets[i]);
			break;
		case PacketType::ServerDeleteEntity:
			this->_resolveServerDeletePacket((ServerDeletePacket*)packets[i]);
			break;
		}
	}

	if (serverUpdate)
		this->_updateWorld(serverUpdate);

	for (size_t i = 0; i < packets.size(); i++) {
		delete packets[i];
	}
}


//ADD ENTITES IN ANY OTHER PLACE THAN ROOT?
void NetClient::_resolveServerSpawnEntityPacket(ServerSpawnEntityPacket* entPacket) {
	nlohmann::json json;
	std::vector<uint8_t> vec;
	for (size_t i = 0; i < entPacket->size; i++) {
		vec.push_back(((uint8_t*)entPacket->data)[i]);
	}
	json = json.from_msgpack(vec);
	Entity* ent = World::newEntity("SERVER CREATED (ERROR)", World::root()).get();
	ent->deserialize(json);
	//ent->setID(entPacket->id);
	this->_IDs[entPacket->id] = ent->id;
}

void NetClient::_resolveServerDeletePacket(ServerDeletePacket* delPacket) {
	std::vector<EntityID> children = World::root()->children;
	for (size_t i = 0; i < children.size(); i++) {
		if (children[i] == delPacket->id) {
			World::getEntity(children[i])->dead = true;
			break;
		}
	}
}

void NetClient::_updateWorld(Packet * updatePacket) {
	std::vector<EntityID> children = World::root()->children;
	ServerUpdatePacket* sup = (ServerUpdatePacket*)updatePacket;
	Hydra::Component::TransformComponent* tc;
	for (size_t k = 0; k < sup->nrOfEntUpdates; k++) {
		if (sup->data[k].entityid == this->_myID)
			continue;
		for (size_t i = 0; i < children.size(); i++) {
			if (children[i] == ((ServerUpdatePacket::EntUpdate)sup->data[k]).entityid) {
				tc = World::getEntity(children[i])->getComponent<Hydra::Component::TransformComponent>().get();
				tc->setPosition(((ServerUpdatePacket::EntUpdate)sup->data[k]).ti.pos);
				tc->setRotation(((ServerUpdatePacket::EntUpdate)sup->data[k]).ti.rot);
				tc->setScale(((ServerUpdatePacket::EntUpdate)sup->data[k]).ti.scale);
				break;
			}
		}
	}
}

void NetClient::_addPlayer(Packet * playerPacket) {
	ServerPlayerPacket* spp = (ServerPlayerPacket*)playerPacket;
	char* c = new char[spp->nameLength + 1];
	memcpy(c, spp->name, spp->nameLength);
	c[spp->nameLength] = '\0';

	Entity* ent = World::newEntity(c, World::root()).get();
	//ent->setID(spp->entID);
	this->_IDs[spp->entID] = ent->id;
	Hydra::Component::TransformComponent* tc = ent->addComponent<Hydra::Component::TransformComponent>().get();
	auto mesh = ent->addComponent<Hydra::Component::MeshComponent>();
	mesh->loadMesh("assets/objects/player.MATTIC");
	tc->setPosition(spp->ti.pos);
	tc->setRotation(spp->ti.rot);
	tc->setScale(spp->ti.scale);
	delete[] c;
}


bool NetClient::initialize(char* ip, int port) {
	SDLNet_Init();
    if(this->_tcp.initialize(ip, port)) {
        this->_myID = 0;
		return true;
    }
	return false;
}

void NetClient::run() {
    {//Receive packets
        this->_resolvePackets();
    }

    //SendUpdate packet
    {
        this->_sendUpdatePacket();
    }

    //Nåt
    {
        
    }
}