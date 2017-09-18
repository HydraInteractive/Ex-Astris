#pragma once

#include <TCPConnection.hpp>
#include <Packets.h>

HYDRA_API class TCPClient : public TCPConnection {
private:
	char* _msg;
	TCPsocket _socket;
	SDLNet_SocketSet _ss;
public:
	HYDRA_API TCPClient();
	HYDRA_API ~TCPClient();
	HYDRA_API bool initiate(IPaddress ip, const char* c);
	HYDRA_API void close();
	HYDRA_API void update();

	HYDRA_API void sendPacket(char* data, int length);
	HYDRA_API std::vector<NetPacket*> receivePacket();
};
