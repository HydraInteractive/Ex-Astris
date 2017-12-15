#pragma once
#include <hydra/ext/api.hpp>

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>

namespace Hydra::Network {
	typedef Hydra::World::EntityID ServerID;
	enum PacketType {
		ServerInitialize,
		ServerInitializePVS,
		ServerFreezePlayer,
		ServerUpdate,
		ServerPlayer,
		ClientUpdate,
		ClientSpawnEntity,
		ServerSpawnEntity,
		ServerDeleteEntity,
		ClientUpdateBullet,
		ServerUpdateBullet,
		ClientShoot,
		ServerShoot,
		ServerPathMap,
		ClientRequestAIInfo,
		ServerAIInfo,
		//..
		MAX_COUNT
	};
	static constexpr const char* PacketTypeName[] = {
		"ServerInitialize",
		"ServerInitializePVS",
		"ServerFreezePlayer",
		"ServerUpdate",
		"ServerPlayer",
		"ClientUpdate",
		"ClientSpawnEntity",
		"ServerSpawnEntity",
		"ServerDeleteEntity",
		"ClientUpdateBullet",
		"ServerUpdateBullet",
		"ClientShoot",
		"ServerShoot",
		"ServerPathMap",
		"ClientRequestAIInfo",
		"ServerAIInfo"
	};

	struct TransformInfo {
		glm::vec3 pos;
		glm::vec3 scale;
		glm::quat rot;
		//INTERPOLATION INFO TEX VELOCITY
	};

	struct Packet {
		PacketType type;
		size_t len;
		ServerID client;

		inline Packet(PacketType type, size_t len) : type(type), len(len) {}
	};

	struct ServerDeleteEntityPacket : public Packet {
		ServerDeleteEntityPacket() : Packet(PacketType::ServerDeleteEntity, sizeof(ServerDeleteEntityPacket)) {}
		ServerID id;
	};

	struct ClientUpdateBulletPacket : public Packet {
		ClientUpdateBulletPacket(size_t size) : Packet(PacketType::ClientUpdateBullet, sizeof(ClientUpdateBulletPacket) + size) {}

		size_t size() const { return len - sizeof(ClientUpdateBulletPacket); }
		char data[0];
	};

	struct ServerUpdateBulletPacket : public Packet {
		ServerUpdateBulletPacket(size_t size) : Packet(PacketType::ServerUpdateBullet, sizeof(ServerUpdateBulletPacket) + size) {}
		ServerID serverPlayerID;

		size_t size() const { return len - sizeof(ServerUpdateBulletPacket); }
		char data[0];
	};

	struct ServerPathMapPacket : public Packet
	{
		ServerPathMapPacket(size_t size) : Packet(PacketType::ServerPathMap, sizeof(ServerPathMapPacket) + size * sizeof(bool)) {}
		size_t size() const { return (len - sizeof(ServerUpdateBulletPacket)) / sizeof(bool); }
		bool data[0];
	};
	///////////////////////////////////////
	struct ClientRequestAIInfoPacket : public Packet
	{
		ClientRequestAIInfoPacket() : Packet(PacketType::ClientRequestAIInfo, sizeof(ClientRequestAIInfoPacket)) {}
		ServerID serverEntityID;
	};
	struct ServerAIInfoPacket : public Packet
	{
		ServerAIInfoPacket(size_t size) : Packet(PacketType::ServerAIInfo, sizeof(ServerAIInfoPacket) + (size * sizeof(int))) {}
		float openList;
		float closedList;
		float pathToEnd;
		int data[0];
	};
	/////////////////////////////////////////
	struct ClientShootPacket : public Packet {
		ClientShootPacket() : Packet(PacketType::ClientShoot, sizeof(ClientShootPacket)) {}
		TransformInfo ti;
		glm::vec3 direction;
	};
	struct ServerShootPacket : public Packet {
		ServerShootPacket() : Packet(PacketType::ServerShoot, sizeof(ServerShootPacket)) {}
		ServerID serverPlayerID;
		TransformInfo ti;
		glm::vec3 direction;
	};

	struct ClientSpawnEntityPacket : public Packet {
		ClientSpawnEntityPacket(size_t size) : Packet(PacketType::ClientSpawnEntity, sizeof(ClientSpawnEntityPacket) + size) {}

		size_t size() const { return len - sizeof(ClientSpawnEntityPacket); }
		char data[0];
	};

	struct ServerSpawnEntityPacket : public Packet {
		ServerSpawnEntityPacket(size_t size) : Packet(PacketType::ServerSpawnEntity, sizeof(ServerSpawnEntityPacket) + size) {}
		Hydra::World::EntityID id;

		size_t size() const { return len - sizeof(ServerSpawnEntityPacket); }
		char data[0];
	};

	struct ServerInitializePacket : public Packet {
		ServerInitializePacket() : Packet(PacketType::ServerInitialize, sizeof(ServerInitializePacket)) {}
		ServerID entityid;
		TransformInfo ti;
	};

	struct ServerInitializePVSPacket : public Packet {
		ServerInitializePVSPacket(size_t size) : Packet(PacketType::ServerInitializePVS, sizeof(ServerInitializePVSPacket) + size) {}

		size_t size() const { return len - sizeof(ServerInitializePVSPacket); }
		char data[0];
	};

	struct ServerFreezePlayerPacket : public Packet {
		ServerFreezePlayerPacket() : Packet(PacketType::ServerFreezePlayer, sizeof(ServerFreezePlayerPacket)) {}
		enum class Action {
			freeze = 0,
			unfreeze,
			noPVS,
			win
		};
		Action action;
	};

	struct ServerUpdatePacket : public Packet {
		ServerUpdatePacket(size_t nrOfEntUpdates) : Packet(PacketType::ServerUpdate, sizeof(ServerUpdatePacket) + nrOfEntUpdates * sizeof(EntUpdate)) {}
		struct EntUpdate {
			ServerID entityid;
			TransformInfo ti;
			int life = 0;
			int animationIndex = 0;
		};

		size_t nrOfEntUpdates() const { return (len - sizeof(ServerUpdatePacket)) / sizeof(EntUpdate); }
		EntUpdate data[0];
	};

	struct ServerPlayerPacket : public Packet {
		ServerPlayerPacket(size_t nameLength) : Packet(PacketType::ServerPlayer, sizeof(ServerPlayerPacket) + nameLength) {}
		TransformInfo ti;
		ServerID entID;

		size_t nameLength() const { return len - sizeof(ServerPlayerPacket); }
		char name[0];
	};

	struct ClientUpdatePacket : public Packet {
		ClientUpdatePacket() : Packet(PacketType::ClientUpdate, sizeof(ClientUpdatePacket)) {}
		TransformInfo ti;
	};

	ClientUpdatePacket* createClientUpdatePacket(Entity* player);
}