#pragma once

#include <TCPConnection.hpp>
#include <Server/TCPHost.hpp>
#include <TCPClient.hpp>
#include <SDL2/SDL_net.h>
#include <hydra/world/world.hpp>
#include <Server/ServerWorld.hpp>

HYDRA_NETWORK_API void startServer(int port);

HYDRA_NETWORK_API class NetworkManager {
private:
	TCPHost* _conn;
	bool _running;
	int _port;
	ServerWorld _serverw;
	//std::map<int, Hydra::World::IEntity*> _netEntities;
	//bool _connected;
	
	HYDRA_NETWORK_API int _generateEntityID();
public:
	HYDRA_NETWORK_API NetworkManager();
	HYDRA_NETWORK_API ~NetworkManager();
	HYDRA_NETWORK_API void update();
	HYDRA_NETWORK_API bool running() { return this->_running; }
	HYDRA_NETWORK_API int getPort() { return this->_port; }

	HYDRA_NETWORK_API bool host(IPaddress ip);
	HYDRA_NETWORK_API void quit();
};
