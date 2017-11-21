#include <server/server.hpp>

Server::Server() {
	this->_running = false;
}

Server::~Server() {
	SDLNet_TCP_Close(this->_sock);
}

bool Server::initialize(int port) {
	this->_clientHandler.initialize(new char[MAX_NETWORK_LENGTH]);
	this->_port = port;

	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, NULL, this->_port) == -1) {
		return this->_running;
	}

	this->_sock = SDLNet_TCP_Open(&ip);
	if (!this->_sock) {
		const char* c = SDLNet_GetError();
		return this->_running;
	}
	this->_running = true;
	return this->_running;
}

int Server::checkForNewClients() {
	TCPsocket new_tcpsock;
	new_tcpsock = SDLNet_TCP_Accept(this->_sock);
	if (!new_tcpsock) {
		return -1;
	}
	else {
		return this->_clientHandler.addNewConnection(new_tcpsock);
	}
	
}

void Server::sendDataToAll(char * data, int length) {
	std::vector<int> clients = this->_clientHandler.getAllClients();
	for (int i = 0; i < clients.size(); i++) {
		this->_clientHandler.sendData(data, length, clients[i]);
	}
}

int Server::sendDataToClient(char * data, int length, int clientID) {
	return this->_clientHandler.sendData(data, length, clientID);
	
}

void Server::sendDataToAllExcept(char * data, int length, int clientID) {
	std::vector<int> clients = this->_clientHandler.getAllClients();
	for (int i = 0; i < clients.size(); i++) {
		if (clients[i] != clientID) {
			this->_clientHandler.sendData(data, length, clients[i]);
		}
	}
}

std::vector<int> Server::getDisconnects() {
	return this->_clientHandler.getDisconnectedClients();
}

std::vector<Packet*> Server::receiveData() {
	return this->_clientHandler.getReceivedData(this->_clientHandler.getActivity());
}



bool Server::isRunning() {
	return this->_running;
}


