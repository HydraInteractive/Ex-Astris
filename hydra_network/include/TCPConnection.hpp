#pragma once
#define NETWORK_MAX_LENGTH 3024
#include <hydra/world/world.hpp>
#include <hydra/ext/api.hpp>
#include <SDL2/SDL_net.h>
		
class HYDRA_NETWORK_API TCPConnection {
protected:
	IPaddress _ip;
public:
	TCPConnection() {}
	virtual ~TCPConnection() {}
	
	virtual bool initiate(IPaddress ip, const char* c) = 0;
	virtual void update() = 0;
	virtual void close() = 0;
};
