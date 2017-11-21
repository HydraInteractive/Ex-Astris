#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/network/tcpclient.hpp>
#include <hydra/world/world.hpp>
#include <hydra/network/packets.hpp>

namespace Hydra::Network {
	typedef Hydra::World::EntityID ServerID;

	struct HYDRA_NETWORK_API NetClient final {
	private:
		static TCPClient _tcp;
		static EntityID _myID;
		static std::map<ServerID, Hydra::World::EntityID> _IDs;
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
