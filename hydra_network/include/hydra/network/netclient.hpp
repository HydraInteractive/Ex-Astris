#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/network/tcpclient.hpp>
#include <hydra/world/world.hpp>
#include <hydra/network/packets.hpp>

namespace Hydra::Network {
	struct HYDRA_NETWORK_API NetClient final {
	public:
		typedef void(*updatePVS_f)(nlohmann::json&& json, void* userdata);
		typedef void(*onWin_f)(void* userdata);
		typedef void(*onNewEntity_f)(Entity* entity, void* userdata);
		typedef void(*updatePathMap_f)(bool* map, void* userdata);
		typedef void(*updatePath_f)(std::vector<glm::vec2>& openList, std::vector<glm::vec2>& closedList, std::vector<glm::vec2>& pathToEnd, void* userdata);

		static updatePVS_f updatePVS;
		static onWin_f onWin;
		static onNewEntity_f onNewEntity;
		static updatePathMap_f updatePathMap;
		static updatePath_f updatePath;
		static void* userdata;
		static bool running;

		static void sendEntity(Hydra::World::EntityID ent);
		static bool initialize(char* ip, int port);
		static void shoot(Hydra::Component::TransformComponent* tc, const glm::vec3& direction);
		static void updateBullet(Hydra::World::EntityID newBulletID);
		static void run();
		static void reset();
		static void enableEntity(Entity* ent);

	private:
		static TCPClient _tcp;
		static Hydra::World::EntityID _myID;
		static std::map<ServerID, Hydra::World::EntityID> _IDs;
		static std::map<ServerID, nlohmann::json> _bullets;

		static void _sendUpdatePacket();
		static void _resolvePackets();
		static void _updateWorld(Packet* updatePacket);
		static void _addPlayer(Packet* playerPacket);
		static void _resolveServerSpawnEntityPacket(ServerSpawnEntityPacket* entPacket);
		static void _resolveServerDeleteEntityPacket(ServerDeleteEntityPacket* delPacket);
	};
}
