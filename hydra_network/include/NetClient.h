#pragma once
#include <Packets.h>
#include <TCPClient.hpp>
#include <NetPlayer.hpp>

class NetClient {
private:
	TCPClient _tcp;
	NetPlayer _player;
	bool _hasSentUpdate;
	void _updateEntity(Hydra::World::IWorld* world, NetEntityInfo* nei);
public:
	HYDRA_API NetClient();
	HYDRA_API ~NetClient();
	HYDRA_API bool initialize(unsigned int port, const char* ip);
	HYDRA_API void update(Hydra::World::IWorld* world);

	HYDRA_API int decodeNewPackets(Hydra::World::IWorld* world);
	HYDRA_API void sendClientUpdatePacket();
};
