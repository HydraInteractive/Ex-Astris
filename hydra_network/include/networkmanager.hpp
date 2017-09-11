#pragma once

#include <../hydra_network/include/TCPConnection.hpp>
#include <../hydra_network/include/TCPHost.hpp>
#include <../hydra_network/include/TCPClient.hpp>
#include <SDL2/SDL_net.h>
#include <hydra/world/world.hpp>

HYDRA_API void startServer(int port);

HYDRA_API class NetworkManager {
private:
	TCPHost* _conn;
	std::vector<TCPsocket> _clients;
	bool _running;
	int _port;
	//std::map<int, Hydra::World::IEntity*> _netEntities;
	//bool _connected;
	HYDRA_API int _generateEntityID();
public:
	HYDRA_API NetworkManager();
	HYDRA_API ~NetworkManager();
	HYDRA_API void update();
	HYDRA_API bool running() { return this->_running; }
	HYDRA_API int getPort() { return this->_port; }

	HYDRA_API bool host(IPaddress ip);
	HYDRA_API void quit();
};