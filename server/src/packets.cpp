#include <server/packets.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <server/server.hpp>
#include <server/gameserver.hpp>

using namespace BarcodeServer;
using namespace Hydra::Network;

using world = Hydra::World::World;

ServerDeletePacket* BarcodeServer::createServerDeletePacket(Hydra::World::EntityID entID) {
	ServerDeletePacket* sdp = new ServerDeletePacket();
	sdp->h.type = PacketType::ServerDeleteEntity;
	sdp->h.len = sizeof(ServerDeletePacket);
	sdp->id = entID;
	return sdp;
}

ServerPlayerPacket* BarcodeServer::createServerPlayerPacket(const std::string& name, const TransformInfo& ti) {
	ServerPlayerPacket* spp;
	volatile int a = sizeof(*spp);
	int size = (sizeof(*spp) + sizeof(char) * name.length());
	spp = (ServerPlayerPacket*)new char[size];
	spp->h.type = PacketType::ServerPlayer;
	spp->nameLength = name.length();
	spp->ti = ti;
	memcpy(spp->name, name.c_str(), name.length());
	spp->h.len = spp->getSize();
	return spp;
}

/*ClientUpdatePacket* BarcodeServer::createClientUpdatePacket(Hydra::World::Entity* player) {
	ClientUpdatePacket* cup;
	cup = new ClientUpdatePacket();
	cup->h.type = PacketType::ClientUpdate;
	cup->h.len = sizeof(ClientUpdatePacket);
	Hydra::Component::TransformComponent* tc = player->getComponent<Hydra::Component::TransformComponent>().get();
	cup->ti.pos = tc->position;
	cup->ti.rot = tc->rotation;
	cup->ti.scale = tc->scale;
	return cup;
	}*/

void BarcodeServer::resolveClientUpdatePacket(ClientUpdatePacket* cup, Hydra::World::EntityID entityID) {
	std::vector<Hydra::World::EntityID> children = World::root()->children;
	if (cup->h.client == 1) {
		int j = 0;
		j++;
	}
	for (size_t i = 0; i < children.size(); i++) {
		if (children[i] == entityID) {
			Hydra::Component::TransformComponent* tc = Hydra::World::World::getEntity(children[i])->getComponent<Hydra::Component::TransformComponent>().get();
			if (tc != nullptr) {
				tc->setPosition(cup->ti.pos);
				tc->setScale(cup->ti.scale);
				tc->setRotation(cup->ti.rot);
			}
			return;
		}
	}
}

Hydra::Network::ServerSpawnEntityPacket* BarcodeServer::createServerSpawnEntity(Hydra::World::Entity* ent) {
	std::vector<uint8_t> data;
	nlohmann::json json;
	ent->serialize(json);
	data = json.to_msgpack(json);

	ServerSpawnEntityPacket* packet = (ServerSpawnEntityPacket*)new char[sizeof(ServerSpawnEntityPacket) + data.size()];
	packet->h.type = PacketType::ServerSpawnEntity;
	packet->id = ent->id;
	packet->size = data.size();
	packet->h.len = packet->getSize();
	memcpy(packet->data, data.data(), data.size());

	return packet;
}

void BarcodeServer::createAndSendPlayerUpdateBulletPacket(Player * p, Server * s) {
	std::vector<uint8_t> vec = p->bullet.to_msgpack(p->bullet);
	ServerUpdateBulletPacket* packet = new ServerUpdateBulletPacket();
	packet->h.type = PacketType::ServerUpdateBullet;
	packet->size = vec.size();
	packet->h.len = packet->getSize();
	packet->serverPlayerID = p->entityid;

	char* result = new char[sizeof(ServerUpdateBulletPacket) + vec.size() * sizeof(uint8_t)];

	memcpy(result, packet, sizeof(ServerUpdateBulletPacket));
	memcpy(result + sizeof(ServerUpdateBulletPacket), vec.data(), vec.size() * sizeof(uint8_t));

	printf("sendDataToAllExcept:\n\ttype: ServerUpdateBullet\n\tlen: %d\n", packet->h.len);
	s->sendDataToAllExcept(result, sizeof(ServerUpdateBulletPacket) + vec.size() * sizeof(uint8_t), p->serverid);
	//_tcp.send(packet, sizeof(ClientSpawnEntityPacket)); // DATA SNED
	//_tcp.send(vec.data(), vec.size() * sizeof(uint8_t)); // DATA SKJICJIK
	//entptr->deserialize(json);
	delete[] result;
	delete packet;
}

void BarcodeServer::createAndSendPlayerShootPacket(Player * p, ClientShootPacket * csp, Server * s) {
	ServerShootPacket* packet = new ServerShootPacket();

	packet->h.len = sizeof(ServerShootPacket);
	packet->h.type = PacketType::ServerShoot;

	packet->direction = csp->direction;
	packet->ti = csp->ti;
	packet->serverPlayerID = p->entityid;

	printf("sendDataToAllExcept:\n\ttype: ServerShoot\n\tlen: %d\n", packet->h.len);
	s->sendDataToAllExcept((char*)packet, sizeof(ServerShootPacket), p->serverid);

	delete packet;
}

Hydra::World::Entity* BarcodeServer::resolveClientSpawnEntityPacket(ClientSpawnEntityPacket* csep, Hydra::World::EntityID id, Server* s) {
	Hydra::World::Entity* ent = World::newEntity("CLIENT CREATED (ERROR)", World::root()).get();
	nlohmann::json json;
	std::vector<uint8_t> data;
	
	//Inefficient
	for (size_t i = 0; i < csep->size; i++) {
		data.push_back(((uint8_t*)csep->data)[i]);
	}

	json = json.from_msgpack(data);
	ent->deserialize(json);
	//ent->setID(world->getFreeID());

	printf("Client Created Entity: \"%s\" with id: %zu\n", ent->name.c_str(), ent->id);
	
	ServerSpawnEntityPacket* packet = new ServerSpawnEntityPacket();
	packet->h.type = PacketType::ServerSpawnEntity;
	packet->id = ent->id;
	packet->size = data.size();
	packet->h.len = packet->getSize();

	char* result = new char[sizeof(ServerSpawnEntityPacket) + data.size() * sizeof(uint8_t)];

	memcpy(result, packet, sizeof(ServerSpawnEntityPacket));
	memcpy(result + sizeof(ServerSpawnEntityPacket), data.data(), data.size() * sizeof(uint8_t));

	printf("sendDataToAll:\n\ttype: ServerShoot\n\tlen: %d\n", packet->h.len);
	s->sendDataToAll(result, sizeof(ServerSpawnEntityPacket) + data.size() * sizeof(uint8_t));

	delete[] result;

	//s->sendDataToAll((char*)packet, sizeof(ServerSpawnEntityPacket));
	//s->sendDataToAll((char*)data.data(), data.size() * sizeof(uint8_t));
	
	delete packet;
	return ent;
}

void BarcodeServer::resolveClientUpdateBulletPacket(ClientUpdateBulletPacket * cubp, nlohmann::json& dest) {
	/*uint8_t* d = reinterpret_cast<uint8_t*>(&cubp->data[0]);
	uint8_t* e = reinterpret_cast<uint8_t*>(&cubp->data[cubp->size]);
	std::vector<uint8_t> v;
	v.resize(cubp->size);
	memcpy(v.data(), d, cubp->size);
	dest = nlohmann::json::from_msgpack(std::move(v));*/
	dest = nlohmann::json::from_msgpack(std::vector<uint8_t>(&cubp->data[0], &cubp->data[cubp->size]));
	printf("Successfully updated bullet.\n");
}

Hydra::World::Entity* BarcodeServer::resolveClientShootPacket(ClientShootPacket * csp, Player * p, Hydra::World::ISystem* physics) {
	Hydra::System::BulletPhysicsSystem* bps = static_cast<Hydra::System::BulletPhysicsSystem*>(physics);
	std::shared_ptr<Hydra::World::Entity> ptr = world::newEntity("BULLET", world::root());
	ptr->deserialize(p->bullet);
	auto otherptr = ptr->getComponent<Hydra::Component::BulletComponent>();
	otherptr->direction = csp->direction;
	auto transform = ptr->getComponent<Hydra::Component::TransformComponent>();
	transform->setPosition(csp->ti.pos);
	transform->setScale(csp->ti.scale);
	transform->setRotation(csp->ti.rot);

	auto r = ptr->getComponent<Hydra::Component::RigidBodyComponent>();
	if (r)
		bps->enable(r.get());

	return ptr.get();
}
