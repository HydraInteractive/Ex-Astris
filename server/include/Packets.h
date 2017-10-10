#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>

class Server;

enum PacketType {
	ServerInitialize,
	ServerUpdate,
	ServerPlayer,
	ClientUpdate,
	ClientSpawnEntity,
	ServerSpawnEntity,
	ServerDeleteEntity,
	//..
};



struct TransformInfo {
	glm::vec3 pos;
	glm::vec3 scale;
	glm::quat rot;
	//INTERPOLATION INFO TEX VELOCITY
};

struct InterpolationInfo {
	// fill with shit
};


struct Header {
	PacketType type;
	int len; // Length of total packet
	int client; // Set manually irrelevant when sending packets
};

struct Packet {
	Header h;
};

struct ServerDeletePacket : Packet {
	int64_t id;
};


struct ClientSpawnEntityPacket : Packet {
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ClientSpawnEntityPacket) + sizeof(char) * size; }
};

struct ServerSpawnEntityPacket : Packet {
	int64_t id;
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ServerSpawnEntityPacket) + sizeof(char) * size; }
};


struct ServerInitializePacket : Packet {
	int64_t entityid;
	TransformInfo ti;
};

struct ServerUpdatePacket : Packet {
	struct EntUpdate {
		int64_t entityid;
		TransformInfo ti;
	};
	size_t nrOfEntUpdates;
	EntUpdate data[0];
};

struct ServerPlayerPacket : Packet {
	TransformInfo ti;
	int64_t entID;
	int nameLength;
	char name[0];

	int getSize() {
		return (sizeof(ServerPlayerPacket) + sizeof(char) * nameLength);
	}
};

struct ClientUpdatePacket : Packet {
	TransformInfo ti;
	InterpolationInfo ipi;
};



ServerDeletePacket* createServerDeletePacket(int64_t entID);
ServerPlayerPacket* createServerPlayerPacket(std::string name, TransformInfo ti);
ClientUpdatePacket* createClientUpdatePacket(std::shared_ptr<Hydra::World::IEntity> player);
void createAndSendServerEntityPacket(Hydra::World::IEntity* ent, Server* s);

void resolveClientUpdatePacket(Hydra::World::IWorld* world, ClientUpdatePacket* cup, int64_t entityID);

std::shared_ptr<Hydra::World::IEntity> resolveClientSpawnEntityPacket(Hydra::World::IWorld* world, ClientSpawnEntityPacket* csep, int64_t entityID, Server* s);