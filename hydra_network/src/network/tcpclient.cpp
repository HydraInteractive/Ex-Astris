#include <hydra/network/tcpclient.hpp>
#include <vector>
#include <SDL2/SDL_net.h>

using namespace Hydra::Network;

TCPClient::TCPClient() {
    this->_connected = false;
}
TCPClient::~TCPClient() {
	close();
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
	if (!_connected)
		return -1;
	((Packet*)data)->h.client = -7;
	return SDLNet_TCP_Send(this->_tcp, data, length);
}

std::vector<Packet*> TCPClient::receiveData() {
	std::vector<Packet*> packets;
	if (!_connected)
		return packets;

	Packet* tmp;
	Packet* tmp2;
	size_t curr = 0;

	size_t offset = 0;
	while (SDLNet_CheckSockets(this->_sset, 0)) {
		if (offset == MAX_NETWORK_LENGTH) {
			close();
			return packets;
		}

		ssize_t lenTmp = SDLNet_TCP_Recv(this->_tcp, this->_msg + offset, MAX_NETWORK_LENGTH - offset) + offset;
		if (lenTmp > 0) {
			size_t len = lenTmp;
			while (curr < len && curr < MAX_NETWORK_LENGTH) {
				tmp = (Packet*)(&(this->_msg[curr]));
				printf("Reading packet: offset: %zu, curr: %zu\n\ttype: %s\n\tlen: %d\n\tclient: %d\n", offset, curr, Hydra::Network::PacketTypeName[tmp->h.type], tmp->h.len, tmp->h.client);
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
		} else {
			close();
			return packets;
		}
	}
	return packets;
}

bool TCPClient::isConnected() {
	return this->_connected;
}

void TCPClient::close() {
	printf("Disconnecting from the server!\n");
	_connected = false;
	SDLNet_FreeSocketSet(_sset);
	_sset = nullptr;
	SDLNet_TCP_Close(_tcp);
	_tcp = nullptr;
	delete[] this->_msg;
}
