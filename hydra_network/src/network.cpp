#include "TCPConnection.hpp"
#include "TCPHost.hpp"
#include "TCPClient.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

TCPHost::TCPHost() {
}

TCPHost::~TCPHost() {
}

bool TCPHost::initiate(IPaddress ip, char* c) {
	this->_ip = ip;
	if (SDLNet_ResolveHost(&_ip, INADDR_ANY, _ip.port) == -1) {
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return false;
	}
	//_ip.host = INADDR_ANY;
	//_ip.port = 3998;
	this->_socket = SDLNet_TCP_Open(&_ip);
	if (!_socket) {
		//printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		const char* c = SDLNet_GetError();
		return false;
	}
	return true;
}

void TCPHost::close() {

}

HYDRA_API TCPsocket TCPHost::checkForClient() {
	//TCPsocket new_tcpsock;
	//new_tcpsock = SDLNet_TCP_Accept(this->_socket);
	//if (!new_tcpsock)
	//	return false;
	//
	//this->_clients.push_back(new_tcpsock);
	//return true;
	TCPsocket new_tcpsock;

	new_tcpsock = SDLNet_TCP_Accept(this->_socket);
	
	return new_tcpsock;
}

void TCPHost::update() {
	return;
}

void TCPHost::sendEntites(std::vector<std::shared_ptr<Hydra::World::IEntity>> list) {
}


//if (SDLNet_ResolveHost(&ip, NULL, 3160) == -1) {
//	printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
//	exit(1);
//}
//socket = SDLNet_TCP_Open(&ip);
//if (!socket) {
//	printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
//	exit(2);
//}



TCPClient::TCPClient() {
}

TCPClient::~TCPClient() {
}

bool TCPClient::initiate(IPaddress ip, char* s) {
	this->_ip = ip;
	if (SDLNet_ResolveHost(&_ip, "localhost", ip.port) == -1) {
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return false;
	}

	this->_socket = SDLNet_TCP_Open(&_ip);
	if (!_socket) {
		std::string str = SDLNet_GetError();
		printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		return false;
	}
	return true;
}

void TCPClient::close() {
}

 void TCPClient::update() { // TODO
	 this->receivePacket();
}

 void TCPClient::receivePacket() {
	int result;
	char msg[NETWORK_MAX_LENGTH];
	
	result = SDLNet_TCP_Recv(_socket, msg, NETWORK_MAX_LENGTH);
	if (result <= 0) {
		// ? QUE ?
	}
}

void TCPClient::sendEntity(std::shared_ptr<Hydra::World::IEntity> entity) {
}


