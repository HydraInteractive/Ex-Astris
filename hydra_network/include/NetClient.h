#pragma once
#include <Packets.h>
#include <TCPClient.hpp>
#include <NetPlayer.hpp>
#include <hydra/ext/api.hpp>

class HYDRA_NETWORK_API NetClient {
private:
	TCPClient _tcp;
	NetPlayer _player;
	bool _hasSentUpdate;
	void _updateEntity(NetEntityInfo* nei);
public:
	 NetClient();
	~NetClient();
	bool initialize(unsigned int port, const char* ip);
	void update();

	int decodeNewPackets();
	void sendClientUpdatePacket();
};
