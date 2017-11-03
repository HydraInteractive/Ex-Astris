#pragma once
#include <hydra/ext/api.hpp>
#include <TCPConnection.hpp>
#include <Packets.h>
#include <SDL2/SDL_net.h>
#define MAX_PLAYERS 4

 class HYDRA_NETWORK_API TCPHost : public TCPConnection {
private:
	SDLNet_SocketSet _clientSocketset;
	TCPsocket* _clients;
	TCPsocket _socket;
	int _clientNr;
public:
	TCPHost();
	~TCPHost();
	bool initiate(IPaddress ip, const char* s = "");
	void close();
	TCPsocket checkForClient();

	void update();

	void sendToClient(char* data, int length, TCPsocket sock);
	void sendToAllClients(char* data, int length);
	void sendToAllExceptOne(char* data, int length, TCPsocket foreverAlone);
	void sendEntites(std::vector<EntityID> list);
	SDLNet_SocketSet getSocketSet();
	std::vector<NetPacket*> receivePacket();
	//void sendEntity(std::shared_ptr<Hydra::World::IEntity> entity);
};
