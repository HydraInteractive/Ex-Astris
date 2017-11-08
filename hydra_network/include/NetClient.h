#pragma once
#include <TCPClient.h>
#include <hydra/world/world.hpp>
#include <Packets.h>

typedef EntityID ServerID;


class NetClient {
private:
    TCPClient _tcp;
    EntityID _myID;
	std::map<ServerID, EntityID> _IDs;
    HYDRA_NETWORK_API void _sendUpdatePacket();
	HYDRA_NETWORK_API void _resolvePackets();
	HYDRA_NETWORK_API void _updateWorld(Packet* updatePacket);
	HYDRA_NETWORK_API void _addPlayer(Packet* playerPacket);
	HYDRA_NETWORK_API void _resolveServerSpawnEntityPacket(ServerSpawnEntityPacket* entPacket);
	HYDRA_NETWORK_API void _resolveServerDeletePacket(ServerDeletePacket* delPacket);
public:
	HYDRA_NETWORK_API void sendEntity(EntityID ent);
	HYDRA_NETWORK_API bool initialize(char* ip, int port);
	HYDRA_NETWORK_API void run();

};