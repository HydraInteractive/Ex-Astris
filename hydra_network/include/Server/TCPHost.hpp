#pragma once
#include <../hydra_network/include/TCPConnection.hpp>
#include <../hydra_network/include/Packets.h>
#include <SDL2\SDL_net.h>
#define MAX_PLAYERS 4

HYDRA_API class TCPHost : public TCPConnection {
private:
	SDLNet_SocketSet _clientSocketset;
	TCPsocket* _clients;
	TCPsocket _socket;
	int _clientNr;
public:
	HYDRA_API TCPHost();
	HYDRA_API ~TCPHost();
	HYDRA_API bool initiate(IPaddress ip, char* s = "");
	HYDRA_API void close();
	HYDRA_API TCPsocket checkForClient();

	HYDRA_API void update();

	HYDRA_API void sendToClient(char* data, int length, TCPsocket sock);
	HYDRA_API void sendToAllClients(char* data, int length);
	HYDRA_API void sendToAllExceptOne(char* data, int length, TCPsocket foreverAlone);
	HYDRA_API void sendEntites(std::vector<std::shared_ptr<Hydra::World::IEntity>> list);
	HYDRA_API SDLNet_SocketSet getSocketSet();
	HYDRA_API NetPacket* receivePacket();
	//void sendEntity(std::shared_ptr<Hydra::World::IEntity> entity);
};