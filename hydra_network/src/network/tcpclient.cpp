#include <hydra/network/tcpclient.hpp>
#include <vector>
#include <SDL2/SDL_net.h>
#define MAX_NETWORK_LENGTH 100000

using namespace Hydra::Network;

TCPClient::TCPClient() {
    this->_connected = false;
}
TCPClient::~TCPClient() {
	if (this->_msg) {
		delete this->_msg;
	}
}

bool TCPClient::initialize(char* ip, int port) {
	IPaddress ipaddr;	
	if(SDLNet_ResolveHost(&ipaddr, ip, port)) {
		return false;
	}
	
	this->_tcp = SDLNet_TCP_Open(&ipaddr);
	if(!this->_tcp) {
		const char* c = SDLNet_GetError();
		return false;
	}
	this->_msg = new char[MAX_NETWORK_LENGTH];
	this->_connected = true;
	this->_sset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(this->_sset, this->_tcp);
	return true;
}

int TCPClient::send(void* data, int length) {
	((Packet*)data)->h.client = -7;
	return SDLNet_TCP_Send(this->_tcp, data, length);
}

std::vector<Packet*> TCPClient::receiveData() {
    std::vector<Packet*> packets;
    Packet* tmp;
    Packet* tmp2;
    int curr = 0;

	static size_t offset = 0;
	while (SDLNet_CheckSockets(this->_sset, 0)) {
		if (offset != 0) {
			offset += 5;
			offset -= 5;
		}
		int len = SDLNet_TCP_Recv(this->_tcp, this->_msg + offset, MAX_NETWORK_LENGTH - offset) + offset;
		if (len > 0) {
			while (curr < len && curr < MAX_NETWORK_LENGTH) {
				tmp = (Packet*)(&(this->_msg[curr]));
				if (!tmp->h.len)
					continue;
				if (curr + tmp->h.len > len) {
					memmove(_msg, _msg + curr, len - curr);
					offset = len - curr;
					curr = 0;
					break;
				}
				curr += tmp->h.len;

				tmp2 = (Packet*)(new char[tmp->h.len]);
				memcpy(tmp2, tmp, tmp->h.len);
				packets.push_back(tmp2);
			}
			if (curr == len)
				offset = 0;
		}
	}
	return packets;
}

bool TCPClient::isConnected() {
	return this->_connected;
}

void TCPClient::close() {
	//CLOSE CODE
	delete[] this->_msg;
}
