#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/system/bulletphysicssystem.hpp>

#ifdef _MSC_VER
#    if (_MSC_VER >= 1800)
#        define __alignas_is_defined 1
#    endif
#    if (_MSC_VER >= 1900)
#        define __alignof_is_defined 1
#    endif
#else
#    include <cstdalign>   // __alignas/of_is_defined directly from the implementation
#endif

#ifdef __alignas_is_defined
#    define ALIGN(X) alignas(X)
#else
#    pragma message("C++11 alignas unsupported :( Falling back to compiler attributes")
#    ifdef __GNUG__
#        define ALIGN(X) __attribute__ ((aligned(X)))
#    elif defined(_MSC_VER)
#        define ALIGN(X) __declspec(align(X))
#    else
#        error Unknown compiler, unknown alignment attribute!
#    endif
#endif

#ifdef __alignof_is_defined
#    define ALIGNOF(X) alignof(x)
#else
#    pragma message("C++11 alignof unsupported :( Falling back to compiler attributes")
#    ifdef __GNUG__
#        define ALIGNOF(X) __alignof__ (X)
#    elif defined(_MSC_VER)
#        define ALIGNOF(X) __alignof(X)
#    else
#        error Unknown compiler, unknown alignment attribute!
#    endif
#endif


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
	ClientShoot,
	ServerUpdateBullet,
	ServerShoot
	//..
};



struct ALIGN(16) TransformInfo {
	glm::vec3 pos;
	glm::vec3 scale;
	glm::quat rot;
	//INTERPOLATION INFO TEX VELOCITY
};

struct ALIGN(16) InterpolationInfo {
	// fill with shit
};


struct ALIGN(16) Header {
	PacketType type;
	int len; // Length of total packet
	int client; // Set manually irrelevant when sending packets
};

struct ALIGN(16) Packet {
	Header h;
};

struct ALIGN(16) ServerDeletePacket : Packet {
	EntityID id;
};

struct ALIGN(16) ClientUpdateBulletPacket : Packet {
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ClientUpdateBulletPacket) + sizeof(char) * size; }
};

struct ALIGN(16) ServerUpdateBulletPacket : Packet {
	EntityID serverPlayerID;
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ServerUpdateBulletPacket) + sizeof(char) * size; }
};

struct ALIGN(16) ServerShootPacket : Packet {
	EntityID serverPlayerID;
	TransformInfo ti;
	glm::vec3 direction;
};

struct ALIGN(16) ClientShootPacket : Packet {
	TransformInfo ti;
	glm::vec3 direction;
};


struct ALIGN(16) ClientSpawnEntityPacket : Packet {
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ClientSpawnEntityPacket) + sizeof(char) * size; }
};

struct ALIGN(16) ServerSpawnEntityPacket : Packet {
	EntityID id;
	size_t size;
	char* data[0];
	inline size_t getSize() { return sizeof(ServerSpawnEntityPacket) + sizeof(char) * size; }
};


struct ALIGN(16) ServerInitializePacket : Packet {
	EntityID entityid;
	TransformInfo ti;
};

struct ALIGN(16) ServerUpdatePacket : Packet {
	struct ALIGN(16) EntUpdate {
		EntityID entityid;
		TransformInfo ti;
	};
	size_t nrOfEntUpdates;
	EntUpdate data[0];
};

struct ALIGN(16) ServerPlayerPacket : Packet {
	TransformInfo ti;
	EntityID entID;
	int nameLength;
	char name[0];

	int getSize() {
		return (sizeof(ServerPlayerPacket) + sizeof(char) * nameLength);
	}
};

struct ALIGN(16) ClientUpdatePacket : Packet {
	TransformInfo ti;
	InterpolationInfo ipi;
};



ServerDeletePacket* createServerDeletePacket(EntityID entID);
ServerPlayerPacket* createServerPlayerPacket(std::string name, TransformInfo ti);
ClientUpdatePacket* createClientUpdatePacket(Entity* player);
void createAndSendServerEntityPacket(Entity* ent, Server* s);
void createAndSendPlayerUpdateBulletPacket(Player* p, Server* s);
void createAndSendPlayerShootPacket(Player* p, ClientShootPacket* csp, Server* s);

void resolveClientUpdatePacket(ClientUpdatePacket* cup, EntityID entityID);
Entity* resolveClientSpawnEntityPacket(ClientSpawnEntityPacket* csep, EntityID entityID, Server* s);
void resolveClientUpdateBulletPacket(ClientUpdateBulletPacket* cubp, nlohmann::json &dest);
Entity* resolveClientShootPacket(ClientShootPacket* csp, Player* p, Hydra::System::BulletPhysicsSystem* bp);
