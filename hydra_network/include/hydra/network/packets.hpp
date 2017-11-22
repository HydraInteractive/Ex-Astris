#pragma once
#include <hydra/ext/api.hpp>

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>

namespace Hydra::Network {
	class Server;

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

	struct ServerDeletePacket : public Packet {
		EntityID id;
	};

	struct ClientUpdateBulletPacket : public Packet {
		size_t size;
		char* data[0];
		inline size_t getSize() { return sizeof(ClientUpdateBulletPacket) + sizeof(char) * size; }
	};
	struct ServerUpdateBulletPacket : public Packet {
		EntityID serverPlayerID;
		size_t size;
		char* data[0];
		inline size_t getSize() { return sizeof(ServerUpdateBulletPacket) + sizeof(char) * size; }
	};


	struct ClientShootPacket : public Packet {
		TransformInfo ti;
		glm::vec3 direction;
	};
	struct ServerShootPacket : public Packet {
		EntityID serverPlayerID;
		TransformInfo ti;
		glm::vec3 direction;
	};



	struct ClientSpawnEntityPacket : public Packet {
		size_t size;
		char* data[0];
		inline size_t getSize() { return sizeof(ClientSpawnEntityPacket) + sizeof(char) * size; }
	};

	struct ServerSpawnEntityPacket : public Packet {
		EntityID id;
		size_t size;
		char* data[0];
		inline size_t getSize() { return sizeof(ServerSpawnEntityPacket) + sizeof(char) * size; }
	};


	struct ServerInitializePacket : public Packet {
		EntityID entityid;
		TransformInfo ti;
	};

	struct ServerUpdatePacket : public Packet {
		struct EntUpdate {
			EntityID entityid;
			TransformInfo ti;
		};
		size_t nrOfEntUpdates;
		EntUpdate data[0];
	};

	struct ServerPlayerPacket : public Packet {
		TransformInfo ti;
		EntityID entID;
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


	ServerPlayerPacket* createServerPlayerPacket(std::string name, TransformInfo ti);

	ClientUpdatePacket* createClientUpdatePacket(Entity* player);

	void resolveClientUpdatePacket(ClientUpdatePacket* cup, Hydra::World::EntityID entityID);
}
