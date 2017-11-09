#include "Packets.h"
#include <hydra/component/transformcomponent.hpp>
#include <Server.h>

ServerDeletePacket * createServerDeletePacket(EntityID entID) {
	ServerDeletePacket* sdp = new ServerDeletePacket();
	sdp->h.type = PacketType::ServerDeleteEntity;
	sdp->h.len = sizeof(ServerDeletePacket);
	sdp->id = entID;
	return sdp;
}

ServerPlayerPacket* createServerPlayerPacket(std::string name, TransformInfo ti) {
	ServerPlayerPacket* spp;
	int size = (sizeof(*spp) + sizeof(char) * name.length());
	spp = (ServerPlayerPacket*)new char[size];
	spp->h.type = PacketType::ServerPlayer;
	spp->nameLength = name.length();
	spp->ti = ti;
	memcpy(spp->name, name.c_str(), name.length());
	spp->h.len = spp->getSize();
	return spp;
}

ClientUpdatePacket* createClientUpdatePacket(Entity* player) {
	ClientUpdatePacket* cup;
	cup = new ClientUpdatePacket();
	cup->h.type = PacketType::ClientUpdate;
	cup->h.len = sizeof(ClientUpdatePacket);
	Hydra::Component::TransformComponent* tc = player->getComponent<Hydra::Component::TransformComponent>().get();
	cup->ti.pos = tc->position;
	cup->ti.rot = tc->rotation;
	cup->ti.scale = tc->scale;
	return cup;
}

void resolveClientUpdatePacket(ClientUpdatePacket* cup, EntityID entityID) {
	std::vector<EntityID> children = World::root()->children;
	if (cup->h.client == 1) {
		int j = 0;
		j++;
	}
	for (size_t i = 0; i < children.size(); i++) {
		if (children[i] == entityID) {
			Hydra::Component::TransformComponent* tc = World::getEntity(children[i])->getComponent<Hydra::Component::TransformComponent>().get();
			if (tc != nullptr) {
				tc->setPosition(cup->ti.pos);
				tc->setScale(cup->ti.scale);
				tc->setRotation(cup->ti.rot);
			}
			return;
		}
	}
}

void createAndSendServerEntityPacket(Entity* ent, Server* s) {

	std::vector<uint8_t> data;
	nlohmann::json json;
	ent->serialize(json);
	data = json.to_msgpack(json);

	ServerSpawnEntityPacket* ssep = new ServerSpawnEntityPacket();
	ssep->h.type = PacketType::ServerSpawnEntity;
	ssep->id = ent->id;
	ssep->size = data.size();
	ssep->h.len = ssep->getSize();

	char* result = new char[sizeof(ServerSpawnEntityPacket) + data.size() * sizeof(uint8_t)];

	memcpy(result, ssep, sizeof(ServerSpawnEntityPacket));
	memcpy(result + sizeof(ServerSpawnEntityPacket), data.data(), data.size() * sizeof(uint8_t));

	s->sendDataToAll(result, sizeof(ServerSpawnEntityPacket) + data.size() * sizeof(uint8_t));

	delete[] result;

	//s->sendDataToAll((char*)ssep, sizeof(ServerSpawnEntityPacket));
	//s->sendDataToAll((char*)data.data(), data.size() * sizeof(uint8_t));

	delete ssep;
}

Entity* resolveClientSpawnEntityPacket(ClientSpawnEntityPacket* csep, EntityID id, Server* s) {
	Entity* ent = World::newEntity("CLIENT CREATED (ERROR)", World::root()).get();
	nlohmann::json json;
	std::vector<uint8_t> data;
	
	//Inefficient
	for (size_t i = 0; i < csep->size; i++) {
		data.push_back(((uint8_t*)csep->data)[i]);
	}

	json = json.from_msgpack(data);
	ent->deserialize(json);
	//ent->setID(world->getFreeID());

	printf("Client Created Entity: \"%s\" with id: %d\n", ent->name.c_str(), ent->id);
	
	ServerSpawnEntityPacket* ssep = new ServerSpawnEntityPacket();
	ssep->h.type = PacketType::ServerSpawnEntity;
	ssep->id = ent->id;
	ssep->size = data.size();
	ssep->h.len = ssep->getSize();

	char* result = new char[sizeof(ServerSpawnEntityPacket) + data.size() * sizeof(uint8_t)];

	memcpy(result, ssep, sizeof(ServerSpawnEntityPacket));
	memcpy(result + sizeof(ServerSpawnEntityPacket), data.data(), data.size() * sizeof(uint8_t));

	s->sendDataToAll(result, sizeof(ServerSpawnEntityPacket) + data.size() * sizeof(uint8_t));

	delete[] result;

	//s->sendDataToAll((char*)ssep, sizeof(ServerSpawnEntityPacket));
	//s->sendDataToAll((char*)data.data(), data.size() * sizeof(uint8_t));
	
	delete ssep;
	return ent;
}