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
	static std::vector<uint8_t> data;
	std::vector<Packet*> packets;
	if (!_connected)
		return packets;

	if (SDLNet_CheckSockets(this->_sset, 0) == 1) {
		ssize_t lenTmp = SDLNet_TCP_Recv(this->_tcp, this->_msg, MAX_NETWORK_LENGTH);
		if (lenTmp <= 0) {
			close();
			return packets;
		}

		size_t offset = 0;
		size_t len = lenTmp;
		while (offset < len) {
			Packet* p = (Packet*)(&(this->_msg[offset]));
			//if (p->h.type != ServerUpdate)
				printf("Reading packet: offset: %zu\n\ttype: %s\n\tlen: %d\n\tclient: %d\n", offset, Hydra::Network::PacketTypeName[p->h.type], p->h.len, p->h.client);
			offset += p->h.len;
			if (offset > len) {
				fprintf(stderr, "PACKET NEEDED MORE DATA THAN AVAILABLE!");
				break;
			}

			Packet* newPacket = (Packet*)(new char[p->h.len]);
			memcpy(newPacket, p, p->h.len);
			packets.push_back(newPacket);
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
