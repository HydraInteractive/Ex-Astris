#include <../hydra_network/include/networkmanager.hpp>

int NetworkManager::_generateEntityID() {
	return int();
}

NetworkManager::NetworkManager() {
	this->_conn = nullptr;
	this->_running = false;
	this->_port = 0;
}

NetworkManager::~NetworkManager() { // TODO
}

void NetworkManager::update() {
	TCPsocket tmpsock = this->_conn->checkForClient();
	if (tmpsock != NULL) {
		this->_clients.push_back(tmpsock);
	}

}

HYDRA_API bool NetworkManager::host(IPaddress ip) {
	if (!this->_running) {
		this->_conn = new TCPHost();

		if (this->_conn->initiate(ip, "") == false) {
			delete this->_conn;
			this->_conn = nullptr;
			return false;
		}
		this->_port = ip.port;
		this->_running = true;
		return true;
	}
}

HYDRA_API void NetworkManager::quit() { // TODO
	return void();
}

HYDRA_API void startServer(int port) {
	NetworkManager nm;
	IPaddress ip;
	ip.host = INADDR_ANY;
	ip.port = port;
	if (nm.host(ip)) {
		while (nm.running()) {
			
			nm.update();

		}
	}
}
