#include "..\include\Server\TCPHost.hpp"

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
	this->_clientSocketset = SDLNet_AllocSocketSet(MAX_PLAYERS);
	this->_clients = new TCPsocket[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; i++) {
		this->_clients[i] = nullptr;
	}
	this->_clientNr = 0;
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
	if (new_tcpsock) {
		SDLNet_TCP_AddSocket(this->_clientSocketset, new_tcpsock);
		for (int i = 0; i < MAX_PLAYERS; i++) {
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

HYDRA_API void TCPHost::sendToClient(char * data, int length, TCPsocket sock) {
	int nr = SDLNet_TCP_Send(sock, data, length);
	nr = nr;
}

HYDRA_API void TCPHost::sendToAllClients(char * data, int length) {
	for (int i = 0; i < this->_clientNr; i++) {
		if (this->_clients[i] != nullptr) {
			SDLNet_TCP_Send(this->_clients[i], data, length);
		}
	}
}

HYDRA_API void TCPHost::sendToAllExceptOne(char * data, int length, TCPsocket foreverAlone) {
	for (int i = 0; i < this->_clientNr; i++) {
		if (this->_clients[i] != nullptr && this->_clients[i] != foreverAlone) {
			SDLNet_TCP_Send(this->_clients[i], data, length);
		}
	}
}

void TCPHost::sendEntites(std::vector<std::shared_ptr<Hydra::World::IEntity>> list) {
}

HYDRA_API SDLNet_SocketSet TCPHost::getSocketSet() {
	return this->_clientSocketset;
}

HYDRA_API NetPacket * TCPHost::receivePacket() {
	
	int result;
	char msg[NETWORK_MAX_LENGTH];
	for (int i = 0; i < this->_clientNr; i++) {
		result = SDLNet_TCP_Recv(this->_clients[i], msg, NETWORK_MAX_LENGTH);
		if (result > 0) {
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
			case PacketType::SpawnEntityClient: //TODO
				return reinterpret_cast<PacketSpawnEntity*>(msg);
				break;
			}
		}
		return nullptr;
	} 
	//else if (numready == -1) {
	//	//printf("Crash");
	//}

	
}
