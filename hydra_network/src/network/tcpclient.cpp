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

	// Delay before dc (depends on frames)
	if (_waitingForData++ == 100) {
		close();
		return packets;
	}

	static uint8_t tmpbuf[0x1000];
	while (SDLNet_CheckSockets(this->_sset, 0) == 1) {
		int64_t lenTmp = SDLNet_TCP_Recv(this->_tcp, tmpbuf, sizeof(tmpbuf));
		if (lenTmp <= 0) {
			close();
			return packets;
		}
		_data.insert(_data.end(), &tmpbuf[0], &tmpbuf[lenTmp]);
		_waitingForData = 0;
	}

	while (_data.size() >= sizeof(Packet)) {
		Packet* p = (Packet*)_data.data();
		if (_data.size() < p->h.len) {
			fprintf(stderr, "PACKET NEEDED MORE DATA THAN AVAILABLE!");
			break;
		}

		//printf("Reading packet:\n\ttype: %s\n\tlen: %zu\n\tclient: %zu\n", Hydra::Network::PacketTypeName[p->h.type], p->h.len, p->h.client);
		Packet* newPacket = (Packet*)(new char[p->h.len]);
		memcpy(newPacket, p, p->h.len);
		packets.push_back(newPacket);
		_data.erase(_data.begin(), _data.begin() + p->h.len);
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
	_data.resize(0);
}
