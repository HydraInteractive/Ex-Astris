#include <hydra/network/netclient.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <hydra/engine.hpp>

using namespace Hydra::Network;
using world = Hydra::World::World;

TCPClient NetClient::_tcp;
EntityID NetClient::_myID;
std::map<ServerID, EntityID> NetClient::_IDs;
bool NetClient::running = false;


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
			glm::vec3 newScale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(tc->getMatrix(), newScale, rotation, translation, skew, perspective);

			ghostobject->ghostObject->setWorldTransform(btTransform(btQuaternion(ghostobject->quatRotation.x, ghostobject->quatRotation.y, ghostobject->quatRotation.z, ghostobject->quatRotation.w), btVector3(translation.x, translation.y, translation.z)));
			
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
		Hydra::Component::TransformComponent* tc = tmp->getComponent<Hydra::Component::TransformComponent>().get();
		cpup.ti.pos = tc->position;
		cpup.ti.scale = tc->scale;
		cpup.ti.rot = tc->rotation;
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
	Entity* ent;
	Packet* serverUpdate = nullptr;
	for (size_t i = 0; i < packets.size(); i++) {
		switch (packets[i]->h.type) {
		case PacketType::ServerInitialize:
			children = world::root()->children;
			for (size_t i = 0; i < children.size(); i++) {
				ent = world::getEntity(children[i]).get();
				if (ent->name == "Player") {
					_myID = children[i];
					break;
				}
			}
			_IDs[((ServerInitializePacket*)packets[i])->entityid] = _myID;
			tc = world::getEntity(_myID)->getComponent<Hydra::Component::TransformComponent>().get();
			tc->setPosition(((ServerInitializePacket*)packets[i])->ti.pos);
			tc->setRotation(((ServerInitializePacket*)packets[i])->ti.rot);
			tc->setScale(((ServerInitializePacket*)packets[i])->ti.scale);
			break;
		case PacketType::ServerUpdate: 
			serverUpdate = packets[i];
			break;
	case PacketType::ServerPlayer:
			_addPlayer(packets[i]);
			break;
		case PacketType::ServerSpawnEntity:
			_resolveServerSpawnEntityPacket((ServerSpawnEntityPacket*)packets[i]);
			break;
		case PacketType::ServerDeleteEntity:
			_resolveServerDeletePacket((ServerDeletePacket*)packets[i]);
			break;
		default:
			break;
		}
	}

	if (serverUpdate)
		_updateWorld(serverUpdate);

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
	mesh->loadMesh("assets/objects/characters/AlienModel.mATTIC");
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
		csp->h.type = PacketType::ClientUpdateBullet;
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

	char* result = new char[sizeof(ClientUpdateBulletPacket) + vec.size() * sizeof(uint8_t)];

	memcpy(result, packet, sizeof(ClientUpdateBulletPacket));
	memcpy(result + sizeof(ClientUpdateBulletPacket), vec.data(), vec.size() * sizeof(uint8_t));

	_tcp.send(result, sizeof(ClientUpdateBulletPacket) + vec.size() * sizeof(uint8_t));
	//_tcp.send(packet, sizeof(ClientSpawnEntityPacket)); // DATA SNED
	//_tcp.send(vec.data(), vec.size() * sizeof(uint8_t)); // DATA SKJICJIK

	entptr->deserialize(json);

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
