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
	HYDRA_API TCPClient();
	HYDRA_API ~TCPClient();
	HYDRA_API bool initialize(char* ip, int port);	
	HYDRA_API int send(void* data, int length);
	HYDRA_API std::vector<Packet*> receiveData();
	HYDRA_API bool isConnected();
	HYDRA_API void close();
};