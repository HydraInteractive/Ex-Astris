#pragma once
#include <TCPClient.h>
#include <hydra/world/world.hpp>
#include <Packets.h>

struct NetPlayer {
    int64_t entID;
    Hydra::World::IEntity* entptr;
};

class NetClient {
private:
    TCPClient _tcp;
    NetPlayer _np;
    HYDRA_API void _sendUpdatePacket();
	HYDRA_API void _resolvePackets(Hydra::World::IWorld* world);
	HYDRA_API void _updateWorld(Hydra::World::IWorld* world, Packet* updatePacket);
	HYDRA_API void _addPlayer(Hydra::World::IWorld* world, Packet* playerPacket);
public:
    HYDRA_API bool initialize(char* ip, int port);
    HYDRA_API void run(Hydra::World::IWorld* world);

};