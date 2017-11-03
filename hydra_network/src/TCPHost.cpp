#include <Server/TCPHost.hpp>

TCPHost::TCPHost() {
}

TCPHost::~TCPHost() {
}

bool TCPHost::initiate(IPaddress ip, const char* c) {
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
		/*const char* c = */SDLNet_GetError();
		return false;
	}
	this->_clientSocketset = SDLNet_AllocSocketSet(MAX_PLAYERS);
	this->_clients = new TCPsocket[MAX_PLAYERS];
	for (size_t i = 0; i < MAX_PLAYERS; i++) {
		this->_clients[i] = nullptr;
	}
	this->_clientNr = 0;
	return true;
}

void TCPHost::close() {

}

HYDRA_NETWORK_API TCPsocket TCPHost::checkForClient() {
	//TCPsocket new_tcpsock;
	//new_tcpsock = SDLNet_TCP_Accept(this->_socket);
	//if (!new_tcpsock)
	//	return false;
	//
	//this->_clients.push_back(new_tcpsock);
	//return true;
	TCPsocket new_tcpsock;

	new_tcpsock = SDLNet_TCP_Accept(this->_socket);
	if (new_tcpsock) {
		SDLNet_TCP_AddSocket(this->_clientSocketset, new_tcpsock);
		for (size_t i = 0; i < MAX_PLAYERS; i++) {
			if (this->_clients[i] == nullptr) {
				this->_clients[i] = new_tcpsock;
				this->_clientNr++;
				break;
			}
		}
	}
	return new_tcpsock;
}

void TCPHost::update() {
	return;
}

HYDRA_NETWORK_API void TCPHost::sendToClient(char * data, int length, TCPsocket sock) {
	int nr = SDLNet_TCP_Send(sock, data, length);
	nr = nr;
}

HYDRA_NETWORK_API void TCPHost::sendToAllClients(char * data, int length) {
	for (int64_t i = 0; i < this->_clientNr; i++) {
		if (this->_clients[i] != nullptr) {
			SDLNet_TCP_Send(this->_clients[i], data, length);
		}
	}
}

HYDRA_NETWORK_API void TCPHost::sendToAllExceptOne(char * data, int length, TCPsocket foreverAlone) {
	for (int64_t i = 0; i < this->_clientNr; i++) {
		if (this->_clients[i] != nullptr && this->_clients[i] != foreverAlone) {
			int nr = SDLNet_TCP_Send(this->_clients[i], data, length);
			nr = nr;
		}
	}
}

void TCPHost::sendEntites(std::vector<EntityID> list) {
}

HYDRA_NETWORK_API SDLNet_SocketSet TCPHost::getSocketSet() {
	return this->_clientSocketset;
}

HYDRA_NETWORK_API std::vector<NetPacket*> TCPHost::receivePacket() {


	/*int nrOfPackets = */SDLNet_CheckSockets(this->_clientSocketset, 0);
	std::vector<NetPacket*> packets;
	int result;
	int packetIndex = 0;
	char msg[NETWORK_MAX_LENGTH];
	for (int64_t j = 0; j < this->_clientNr; j++) {
		if (SDLNet_SocketReady(this->_clients[j])) {
			result = SDLNet_TCP_Recv(this->_clients[j], (char*)(msg + packetIndex), NETWORK_MAX_LENGTH - packetIndex);
			if (result > 0) {
				NetPacket* np = (NetPacket*)(char*)(msg +  packetIndex);
				switch (np->header.type) {
				case PacketType::HelloWorld:
					packets.push_back(np);
					packetIndex += sizeof(PacketHelloWorld);
					break;
				case PacketType::ChangeID: //TODO
					packets.push_back(np);
					packetIndex += sizeof(PacketChangeID);
					break;
				case PacketType::ClientUpdate: //TODO
					packets.push_back(np);
					packetIndex += sizeof(PacketClientUpdate);
					break;
				case PacketType::SpawnEntityClient: //TODO
					packets.push_back(np);
					packetIndex += sizeof(PacketSpawnEntityClient);
					break;
				default:
					break;
				}
			}
		}
	}
	return packets;
	//else if (numready == -1) {
	//	//printf("Crash");
	//}

	
}
