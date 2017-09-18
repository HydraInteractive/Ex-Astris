#pragma once
#define NETWORK_MAX_LENGTH 3024
#include <hydra/world/world.hpp>
#include <hydra/ext/api.hpp>
#include <SDL2/SDL_net.h>

HYDRA_API class TCPConnection {
protected:
	IPaddress _ip;
public:
	TCPConnection() {}
	virtual ~TCPConnection() {}
	
	virtual bool initiate(IPaddress ip, const char* c) = 0;
	virtual void update() = 0;
	virtual void close() = 0;
	//virtual void sendEntity(std::shared_ptr<Hydra::World::IEntity> entity) = 0;
	//virtual void receiveEntity() = 0;
};


//enum {
//
//};

//if (SDLNet_ResolveHost(&ip, NULL, 3160) == -1) {
//	printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
//	exit(1);
//}
//socket = SDLNet_TCP_Open(&ip);
//if (!socket) {
//	printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
//	exit(2);
//}
