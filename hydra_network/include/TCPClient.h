#pragma once
#include <SDL2\SDL_net.h>
#include <Packets.h>

class TCPClient {
private:
	SDLNet_SocketSet _sset;
	TCPsocket _tcp;
	char* _msg;
	bool _connected;
public:
	HYDRA_NETWORK_API TCPClient();
	HYDRA_NETWORK_API ~TCPClient();
	HYDRA_NETWORK_API bool initialize(char* ip, int port);
	HYDRA_NETWORK_API int send(void* data, int length);
	HYDRA_NETWORK_API std::vector<Packet*> receiveData();
	HYDRA_NETWORK_API bool isConnected();
	HYDRA_NETWORK_API void close();
};