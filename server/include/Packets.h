#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>

class Server;
struct Player;

enum PacketType {
	ServerInitialize,
	ServerUpdate,
	ServerPlayer,
	ClientUpdate,
	ClientSpawnEntity,
	ServerSpawnEntity,
	ServerDeleteEntity,
	ClientUpdateBullet,
	ClientShoot
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
	EntityID id;
};

struct 	ClientUpdateBulletPacket : Packet {
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ClientUpdateBulletPacket) + sizeof(char) * size; }
};

struct ClientShootPacket : Packet {
	TransformInfo ti;
	glm::vec3 direction;
};


struct ClientSpawnEntityPacket : Packet {
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ClientSpawnEntityPacket) + sizeof(char) * size; }
};

struct ServerSpawnEntityPacket : Packet {
	EntityID id;
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ServerSpawnEntityPacket) + sizeof(char) * size; }
};


struct ServerInitializePacket : Packet {
	EntityID entityid;
	TransformInfo ti;
};

struct ServerUpdatePacket : Packet {
	struct EntUpdate {
		EntityID entityid;
		TransformInfo ti;
	};
	size_t nrOfEntUpdates;
	EntUpdate data[0];
};

struct ServerPlayerPacket : Packet {
	TransformInfo ti;
	EntityID entID;
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



ServerDeletePacket* createServerDeletePacket(EntityID entID);
ServerPlayerPacket* createServerPlayerPacket(std::string name, TransformInfo ti);
ClientUpdatePacket* createClientUpdatePacket(Entity* player);
void createAndSendServerEntityPacket(Entity* ent, Server* s);

void resolveClientUpdatePacket(ClientUpdatePacket* cup, EntityID entityID);
Entity* resolveClientSpawnEntityPacket(ClientSpawnEntityPacket* csep, EntityID entityID, Server* s);
void resolveClientUpdateBulletPacket(ClientUpdateBulletPacket* cubp, nlohmann::json &dest);
void resolveClientShootPacket(ClientShootPacket* csp, Player* p);