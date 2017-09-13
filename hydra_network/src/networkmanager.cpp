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

	//---------------CHECK FOR INCOMING CLIENTS/CONNECTIONS --------------
	{
		TCPsocket tmpsock = this->_conn->checkForClient();
		if (tmpsock != NULL) {
			//ADD PLAYER/SOCKET / SEND HELLOWORLDPACKET
			PacketHelloWorld phw;
			phw.yourID = 0;
		}
	}

	
	//---------------CHECK FOR INCOMING PACKETS (Handle one packet per update())--------------
	{
		int nrOfPackets = SDLNet_CheckSockets(this->_conn->getSocketSet(), 0);
		if (nrOfPackets > 0) {
			for (int i = 0; i < nrOfPackets; i++) {
				NetPacket* np = this->_conn->receivePacket();
				if (np != nullptr) {
					//DECODE / DO ACTIONS THAT ARE RELEVANT :S
					switch (np->header.type) {
					case PacketType::ChangeID:
						break;
					case PacketType::ClientUpdate:
						break;
					case PacketType::SpawnEntity:
						break;
					}



				}
			}
		}
	}



	//---------------UPDATE WORLD--------------
	{
	}


	//---------------SEND UPDATED WORLD TO CLIENTS---------------
	{
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
