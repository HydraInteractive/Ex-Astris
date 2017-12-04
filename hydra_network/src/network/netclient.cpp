#include <hydra/network/netclient.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <hydra/engine.hpp>

using namespace Hydra::Network;
using world = Hydra::World::World;

bool NetClient::running = false;

TCPClient NetClient::_tcp;
EntityID NetClient::_myID;
std::map<ServerID, EntityID> NetClient::_IDs;
std::map<ServerID, nlohmann::json> NetClient::_bullets;

void enableEntity(Entity* ent) {
	if (!ent)
		return;

	Hydra::System::BulletPhysicsSystem* bulletsystem = (Hydra::System::BulletPhysicsSystem*)Hydra::IEngine::getInstance()->getState()->getPhysicsSystem();
	Hydra::Component::RigidBodyComponent* rgb = ent->getComponent<Hydra::Component::RigidBodyComponent>().get();
	if (rgb)
		bulletsystem->enable(rgb);
	
	
	auto tc = ent->getComponent<TransformComponent>();
	if (tc) {
		auto ghostobject = ent->getComponent<GhostObjectComponent>();
		if (ghostobject) {
			ghostobject->updateWorldTransform();
			bulletsystem->enable(static_cast<Hydra::Component::GhostObjectComponent*>(ghostobject.get()));
		}
	}
	std::vector<EntityID> children = ent->children;
	for (size_t i = 0; i < children.size(); i++) {
		enableEntity(world::getEntity(children[i]).get());
	}
}

void NetClient::_sendUpdatePacket() {
	Entity* tmp = world::getEntity(_myID).get();
	if (tmp) {
		ClientUpdatePacket cpup;
		auto tc = tmp->getComponent<Hydra::Component::TransformComponent>();
		auto cc = tmp->getComponent<Hydra::Component::CameraComponent>();
		cpup.ti.pos = tc->position;
		cpup.ti.scale = tc->scale;
		//cpup.ti.rot = tc->rotation;
		cpup.ti.rot = glm::angleAxis(cc->cameraYaw - 1.6f /* Player model fix */, glm::vec3(0, -1, 0));
		cpup.h.type = PacketType::ClientUpdate;
		cpup.h.len = sizeof(ClientUpdatePacket);

		_tcp.send(&cpup, cpup.h.len);
	}
}
void NetClient::sendEntity(EntityID ent) {
	nlohmann::json json;
	Entity* entptr = world::getEntity(ent).get();
	entptr->serialize(json);
	std::vector<uint8_t> vec = json.to_msgpack(json);
	ClientSpawnEntityPacket* packet = new ClientSpawnEntityPacket();
	packet->h.type = PacketType::ClientSpawnEntity;
	packet->size = vec.size();
	packet->h.len = packet->getSize();

	char* result = new char[sizeof(ClientSpawnEntityPacket) + vec.size() * sizeof(uint8_t)];
	
	memcpy(result, packet, sizeof(ClientSpawnEntityPacket));
	memcpy(result + sizeof(ClientSpawnEntityPacket), vec.data(), vec.size() * sizeof(uint8_t));

	_tcp.send(result, sizeof(ClientSpawnEntityPacket) + vec.size() * sizeof(uint8_t));
	//_tcp.send(packet, sizeof(ClientSpawnEntityPacket)); // DATA SNED
	//_tcp.send(vec.data(), vec.size() * sizeof(uint8_t)); // DATA SKJICJIK

	delete[] result;
	delete packet;
}

void NetClient::_resolvePackets() {
	std::vector<Packet*> packets = _tcp.receiveData();
	Hydra::Component::TransformComponent* tc;
	std::vector<EntityID> children;
	Entity* ent = nullptr;
	Packet* serverUpdate = nullptr;
	for (size_t i = 0; i < packets.size(); i++) {
		auto& p = packets[i];
		switch (p->h.type) {
		case PacketType::ServerInitialize:
			children = world::root()->children;
			for (size_t i = 0; i < children.size(); i++) {
				ent = world::getEntity(children[i]).get();
				if (ent->name == "Player") {
					_myID = children[i];
					break;
				}
			}
			_IDs[((ServerInitializePacket*)p)->entityid] = _myID;
			tc = ent->getComponent<Hydra::Component::TransformComponent>().get();
			tc->setPosition(((ServerInitializePacket*)p)->ti.pos);
			tc->setRotation(((ServerInitializePacket*)p)->ti.rot);
			tc->setScale(((ServerInitializePacket*)p)->ti.scale);

			if (auto rb = ent->getComponent<Hydra::Component::RigidBodyComponent>(); rb)
				rb->refreshTransform();
			if (auto go = ent->getComponent<Hydra::Component::GhostObjectComponent>(); go)
				go->updateWorldTransform();
			break;
		case PacketType::ServerUpdate:
			serverUpdate = p;
			break;
		case PacketType::ServerPlayer:
			_addPlayer(p);
			break;
		case PacketType::ServerSpawnEntity:
			_resolveServerSpawnEntityPacket((ServerSpawnEntityPacket*)p);
			break;
		case PacketType::ServerDeleteEntity:
			_resolveServerDeletePacket((ServerDeletePacket*)p);
			break;
		case PacketType::ServerUpdateBullet: {
			auto ubp = (ServerUpdateBulletPacket*)p;
			_bullets[ubp->serverPlayerID] = nlohmann::json::from_msgpack(std::vector<uint8_t>(&ubp->data[0], &ubp->data[ubp->size]));
			break;
		}
		case PacketType::ServerShoot: {
			auto ss = (ServerShootPacket*)p;
			auto b = world::newEntity("EXTERNAL BULLET", world::root());
			b->deserialize(_bullets[ss->serverPlayerID]);
			auto bc = b->getComponent<Hydra::Component::BulletComponent>();
			bc->direction = ss->direction;
			auto tc = b->getComponent<Hydra::Component::TransformComponent>();
			tc->position = ss->ti.pos;
			tc->scale = ss->ti.scale;
			tc->rotation = ss->ti.rot;
			auto r = b->getComponent<Hydra::Component::RigidBodyComponent>();
			if (r)
				static_cast<Hydra::System::BulletPhysicsSystem*>(Hydra::IEngine::getInstance()->getState()->getPhysicsSystem())->enable(r.get());
			break;
		}
		case PacketType::ServerFreezePlayer: {
			auto sfp = (ServerFreezePlayerPacket*)p;
			for (auto p : Hydra::Component::PlayerComponent::componentHandler->getActiveComponents())
				((Hydra::Component::PlayerComponent*)p.get())->frozen = sfp->action == ServerFreezePlayerPacket::Action::freeze;
			//TODO Loading screen;
			break;
		}
		default:
			printf("UNKNOWN PACKET: %s\n", (p->h.type < PacketType::MAX_COUNT ? PacketTypeName[p->h.type] : "(unk)"));
			break;
		}
	}

	if (serverUpdate)
		_updateWorld(serverUpdate);

	for (size_t i = 0; i < packets.size(); i++)
		delete packets[i];
}


//ADD ENTITES IN ANY OTHER PLACE THAN ROOT?
void NetClient::_resolveServerSpawnEntityPacket(ServerSpawnEntityPacket* entPacket) {
	nlohmann::json json;
	std::vector<uint8_t> vec;
	for (size_t i = 0; i < entPacket->size; i++) {
		vec.push_back(((uint8_t*)entPacket->data)[i]);
	}
	json = json.from_msgpack(vec);
	Entity* ent = world::newEntity("SERVER CREATED (ERROR)", world::root()).get();
	ent->deserialize(json);
	//ent->setID(entPacket->id);
	_IDs[entPacket->id] = ent->id;

	enableEntity(ent);
	
}

void NetClient::_resolveServerDeletePacket(ServerDeletePacket* delPacket) {
	std::vector<EntityID> children = world::root()->children;
	for (size_t i = 0; i < children.size(); i++) {
		if (children[i] == _IDs[delPacket->id]) {
			world::getEntity(children[i])->dead = true;
			break;
		}
	}
}

void NetClient::_updateWorld(Packet * updatePacket) {
	std::vector<EntityID> children = world::root()->children;
	ServerUpdatePacket* sup = (ServerUpdatePacket*)updatePacket;
	Hydra::Component::TransformComponent* tc;
	for (size_t k = 0; k < sup->nrOfEntUpdates; k++) {
		if (_IDs[sup->data[k].entityid] == _myID)
			continue;
		else if (_IDs[sup->data[k].entityid] == 0) {
			printf("Error updating entity: %zu\n", _IDs[sup->data[k].entityid]);

			auto ent = world::newEntity("ERROR: UNKOWN ENTITY", world::root());
			_IDs[sup->data[k].entityid] = ent->id;
			auto mesh = ent->addComponent<MeshComponent>();
			mesh->loadMesh("assets/objects/characters/PlayerModel.mATTIC");
			auto transform = ent->addComponent<TransformComponent>();
			transform->position = { ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.x, ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.y - 2, ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.z };

			continue;
		}
		for (size_t i = 0; i < children.size(); i++) {
			if (children[i] == _IDs[((ServerUpdatePacket::EntUpdate&)sup->data[k]).entityid]) {
				tc = world::getEntity(children[i])->getComponent<Hydra::Component::TransformComponent>().get();
				if (tc) {
					tc->position = { ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.x, ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.y - 2, ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.z };
					tc->setRotation(((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.rot);
					tc->setScale(((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.scale);
				}
				else {
					auto entity = world::newEntity("ERROR: NETWORK ENTITY UNKOWN", world::root());
					auto mesh = entity->addComponent<Hydra::Component::MeshComponent>().get();
					mesh->meshFile = "assets/objects/characters/AlienModel.mATTIC";
					tc = entity->addComponent<Hydra::Component::TransformComponent>().get();
					tc->position = { ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.x, ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.y - 2, ((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.pos.z };
					tc->setRotation(((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.rot);
					tc->setScale(((ServerUpdatePacket::EntUpdate&)sup->data[k]).ti.scale);

				}
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

	Entity* ent = world::newEntity(c, world::root()).get();
	//ent->setID(spp->entID);
	_IDs[spp->entID] = ent->id;
	Hydra::Component::TransformComponent* tc = ent->addComponent<Hydra::Component::TransformComponent>().get();
	auto mesh = ent->addComponent<Hydra::Component::MeshComponent>();
	mesh->loadMesh("assets/objects/characters/PlayerModel.mATTIC");
	tc->setPosition(spp->ti.pos);
	tc->setRotation(spp->ti.rot);
	tc->setScale(spp->ti.scale);
	delete[] c;
}


bool NetClient::initialize(char* ip, int port) {
	SDLNet_Init();
	if(_tcp.initialize(ip, port)) {
		_myID = 0;
		NetClient::running = true;
		return true;
	}
	NetClient::running = false;
	return false;
}

void NetClient::shoot(Hydra::Component::TransformComponent * tc, const glm::vec3& direction) {
	if (NetClient::_tcp.isConnected()) {
		ClientShootPacket* csp = new ClientShootPacket();

		csp->h.len = sizeof(ClientShootPacket);
		csp->h.type = PacketType::ClientShoot;
		csp->direction = direction;
		csp->ti.pos = tc->position;
		csp->ti.scale = tc->scale;
		csp->ti.rot = tc->rotation;

		NetClient::_tcp.send(csp, sizeof(ClientShootPacket));

		delete csp;
	}
}

void NetClient::updateBullet(EntityID newBulletID) {
	nlohmann::json json;
	Entity* entptr = world::getEntity(newBulletID).get();
	entptr->serialize(json);
	std::vector<uint8_t> vec = json.to_msgpack(json);
	ClientUpdateBulletPacket* packet = new ClientUpdateBulletPacket();
	packet->h.type = PacketType::ClientUpdateBullet;
	packet->size = vec.size();
	packet->h.len = packet->getSize();

	char* result = new char[sizeof(ClientUpdateBulletPacket) + vec.size()];

	memcpy(result, packet, sizeof(ClientUpdateBulletPacket));
	memcpy(result + sizeof(ClientUpdateBulletPacket), vec.data(), vec.size());

	_tcp.send(result, sizeof(ClientUpdateBulletPacket) + vec.size());
	//_tcp.send(packet, sizeof(ClientSpawnEntityPacket)); // DATA SNED
	//_tcp.send(vec.data(), vec.size() * sizeof(uint8_t)); // DATA SKJICJIK

	delete[] result;
	delete packet;
}


void NetClient::run() {
    {//Receive packets
        _resolvePackets();
    }

    //SendUpdate packet
    {
        _sendUpdatePacket();
    }

    //Nåt
    {
        //du gillar sovpotatisar no?
    }
}

void NetClient::reset() {
	if (!running)
		return;
	_tcp.close();
	_IDs.clear();
	running = false;
}
