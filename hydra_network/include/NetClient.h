#pragma once
#include <../hydra_network/include/TCPClient.h>
#include <hydra/world/world.hpp>
#include <../hydra_network/include/Packets.h>

typedef EntityID ServerID;

namespace Network {
	struct HYDRA_NETWORK_API NetClient final {
	private:
		static TCPClient _tcp;
		static EntityID _myID;
		static std::map<ServerID, EntityID> _IDs;
		static void _sendUpdatePacket();
		static void _resolvePackets();
		static void _updateWorld(Packet* updatePacket);
		static void _addPlayer(Packet* playerPacket);
		static void _resolveServerSpawnEntityPacket(ServerSpawnEntityPacket* entPacket);
		static void _resolveServerDeletePacket(ServerDeletePacket* delPacket);
	public:
		static bool running;

		static void sendEntity(EntityID ent);
		static bool initialize(char* ip, int port);
		static void shoot(Hydra::Component::TransformComponent* tc, glm::vec3 direction);
		static void updateBullet(EntityID newBulletID);
		static void run();
	};
}