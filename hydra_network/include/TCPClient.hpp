#pragma once
#include <hydra/ext/api.hpp>
#include <TCPConnection.hpp>
#include <Packets.h>

class HYDRA_NETWORK_API TCPClient : public TCPConnection {
private:
	char* _msg;
	TCPsocket _socket;
	SDLNet_SocketSet _ss;
public:
	TCPClient();
	~TCPClient();
	bool initiate(IPaddress ip, const char* c);
	void close();
	void update();

	void sendPacket(char* data, int length);
	std::vector<NetPacket*> receivePacket();
};
