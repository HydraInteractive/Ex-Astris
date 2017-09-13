#pragma once
#include <hydra\component\transformcomponent.hpp>

enum class PacketType {
	HelloWorld,
	ChangeID,
	ClientUpdate,
	SpawnEntity
	// ...
};

class NetPacket {
public:
	PacketHeader header;
};

struct PacketHeader {
	PacketType type;
};


// When a player connects, or the world changes
struct PacketHelloWorld : public NetPacket {
	int64_t yourID;
	//char[] MapData;
public:
	int64_t getID() { return this->yourID; }
};

// Entity changes ID
// x < 0 = Local ID
// x > 0 = Server ID
struct PacketChangeID : public NetPacket {
	int64_t oldID; // < 0
	int64_t ID; // > 0
};

// Client sends this
struct PacketClientUpdate : public NetPacket{
	int64_t owner;
	glm::vec3 position;
	glm::quat rotation;
	// ...
};

struct PacketSpawnEntity : public NetPacket {
	int64_t id;
	std::string json;
};