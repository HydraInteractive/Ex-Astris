#include <Server/networkmanager.hpp>
#include <hydra/world/world.hpp>
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
			phw.yourID = this->_serverw.addPlayer(glm::vec3(0, 0, 0), glm::quat(), tmpsock);
			this->_conn->sendToClient((char*)(&phw), sizeof(phw), tmpsock);
			//SAVE PlAYER ID SOMEWHERE COLD

			this->_serverw.sendPlayers(this->_conn, phw.yourID);
			//Update new player about current world?
			//TMP FIX
			//PacketSpawnEntityServer pse2;
			//pse2.header.type = PacketType::SpawnEntityServer;
			//pse2.id = 1;
			//pse2.ti.position = glm::vec3(0, 0, 0);
			//pse2.ti.scale = glm::vec3(1, 1, 1);
			//pse2.ti.rot = glm::quat();
			
			//this->_conn->sendToClient((char*)(&pse2), sizeof(pse2), tmpsock);
			//TMP FIX
			//??

			//Create new player for other players
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
		std::vector<NetPacket*> packets = this->_conn->receivePacket();
		for (size_t i = 0; i < packets.size(); i++) {
			NetPacket* np = packets[i];
			if (np != nullptr) {

				//DECODE / DO ACTIONS THAT ARE RELEVANT :S
				TransformInfo pi;
				switch (np->header.type) {
				case PacketType::ChangeID:
					break;
				case PacketType::ClientUpdate:
					if (((PacketClientUpdate*)np)->owner == 1) {
						pi.position = pi.position;
					}
					pi.position = ((PacketClientUpdate*)np)->ti.position;
					pi.rot = ((PacketClientUpdate*)np)->ti.rot;
					pi.scale = ((PacketClientUpdate*)np)->ti.scale;
					this->_serverw.updateEntityTransform(pi, ((PacketClientUpdate*)np)->owner);
					break;
				default:
					break;
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
	return false;
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
