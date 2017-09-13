#include "TCPConnection.hpp"
#include "TCPClient.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>




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
	if (SDLNet_ResolveHost(&_ip, s, ip.port) == -1) {
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

 HYDRA_API void TCPClient::sendPacket(char * data, int length) {
	if (this->_socket) {
		 SDLNet_TCP_Send(this->_socket, data, length);
	}
 }

 NetPacket* TCPClient::receivePacket() {
	int result;
	char msg[NETWORK_MAX_LENGTH];
	result = SDLNet_TCP_Recv(_socket, msg, NETWORK_MAX_LENGTH);
	if (result <= 0) {
		NetPacket* np = (NetPacket*)msg;
		switch (np->header.type) {
		case PacketType::HelloWorld:
			return reinterpret_cast<PacketHelloWorld*>(msg);
			break;
		case PacketType::ChangeID: //TODO
			return reinterpret_cast<PacketChangeID*>(msg);
			break;
		case PacketType::ClientUpdate: //TODO
			return reinterpret_cast<PacketClientUpdate*>(msg);
			break;
		case PacketType::SpawnEntity: //TODO
			return reinterpret_cast<PacketSpawnEntity*>(msg);
			break;
		}
	}
}


