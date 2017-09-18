#include <../hydra_network/include/Server/networkmanager.hpp>
#include <hydra\world\world.hpp>
#include <thread>
#include <chrono>

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

	std::this_thread::sleep_for(std::chrono::milliseconds(1000/10));

	//---------------CHECK FOR INCOMING CLIENTS/CONNECTIONS --------------
	{
		TCPsocket tmpsock = this->_conn->checkForClient();
		if (tmpsock != NULL) {
			//ADD PLAYER/SOCKET / SEND HELLOWORLDPACKET
			PacketHelloWorld phw;
			phw.header.type = PacketType::HelloWorld;
			phw.yourID = this->_serverw.addPlayer(glm::vec3(0, 0, 0), glm::quat());

			this->_conn->sendToClient((char*)(&phw), sizeof(phw), tmpsock);
			//SAVE PlAYER ID SOMEWHERE COLD

			PacketSpawnEntityServer pse;
			pse.header.type = PacketType::SpawnEntityServer;
			pse.id = phw.yourID;
			pse.ti.position = glm::vec3(0, 0, 0);
			pse.ti.scale = glm::vec3(1, 1, 1);
			pse.ti.rot = glm::quat();

			this->_conn->sendToAllExceptOne((char*)(&pse), sizeof(pse), tmpsock);
		}
	}

	
	//---------------CHECK FOR INCOMING PACKETS (handle every packet before proceeding)--------------
	{
		int nrOfPackets = SDLNet_CheckSockets(this->_conn->getSocketSet(), 0);
		if (nrOfPackets > 0) {
			for (int i = 0; i < nrOfPackets; i++) {
				NetPacket* np = this->_conn->receivePacket();
				if (np != nullptr) {
					//DECODE / DO ACTIONS THAT ARE RELEVANT :S
					TransformInfo pi;
					switch (np->header.type) {
					case PacketType::ChangeID:
						break;
					case PacketType::ClientUpdate:
						pi.position = ((PacketClientUpdate*)np)->position;
						pi.rot = ((PacketClientUpdate*)np)->rotation;
						this->_serverw.updateEntityTransform(pi, ((PacketClientUpdate*)np)->owner);
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
		this->_serverw.sendCurrentWorld(this->_conn);
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
		this->_serverw.initialize();
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
