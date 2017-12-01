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
		ServerShoot
		//..
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
		"ServerShoot"
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
		size_t len; // Length of total packet
		ServerID client; // Set manually irrelevant when sending packets
	};

	struct Packet {
		Header h;
	};

	struct ServerDeletePacket : public Packet {
		ServerID id;
	};

	struct ClientUpdateBulletPacket : public Packet {
		size_t size;
		char data[0];
		inline size_t getSize() { return sizeof(ClientUpdateBulletPacket) + sizeof(char) * size; }
	};
	struct ServerUpdateBulletPacket : public Packet {
		ServerID serverPlayerID;
		size_t size;
		char data[0];
		inline size_t getSize() { return sizeof(ServerUpdateBulletPacket) + sizeof(char) * size; }
	};


	struct ClientShootPacket : public Packet {
		TransformInfo ti;
		glm::vec3 direction;
	};
	struct ServerShootPacket : public Packet {
		ServerID serverPlayerID;
		TransformInfo ti;
		glm::vec3 direction;
	};



	struct ClientSpawnEntityPacket : public Packet {
		size_t size;
		char data[0];
		inline size_t getSize() { return sizeof(ClientSpawnEntityPacket) + sizeof(char) * size; }
	};

	struct ServerSpawnEntityPacket : public Packet {
		ServerID id;
		size_t size;
		char data[0];
		inline size_t getSize() { return sizeof(ServerSpawnEntityPacket) + sizeof(char) * size; }
	};


	struct ServerInitializePacket : public Packet {
		ServerID entityid;
		TransformInfo ti;
	};

	struct ServerInitializePVSPacket : public Packet {
		size_t size;
		char data[0];
	};

	struct ServerFreezePlayerPacket : public Packet {
		enum class Action {
			freeze = 0,
			unfreeze
		};
		Action action;
	};

	struct ServerUpdatePacket : public Packet {
		struct EntUpdate {
			ServerID entityid;
			TransformInfo ti;
		};
		size_t nrOfEntUpdates;
		EntUpdate data[0];
	};

	struct ServerPlayerPacket : public Packet {
		TransformInfo ti;
		ServerID entID;
		int nameLength;
		char name[0];

		int getSize() {
			return (sizeof(ServerPlayerPacket) + sizeof(char) * nameLength);
		}
	};

	struct ClientUpdatePacket : public Packet {
		TransformInfo ti;
		InterpolationInfo ipi;
	};

	ClientUpdatePacket* createClientUpdatePacket(Entity* player);
}
