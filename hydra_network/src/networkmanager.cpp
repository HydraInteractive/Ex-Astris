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

	std::this_thread::sleep_for(std::chrono::milliseconds(1000/30));

	//---------------CHECK FOR INCOMING CLIENTS/CONNECTIONS --------------
	{
		TCPsocket tmpsock = this->_conn->checkForClient();
		if (tmpsock != NULL) {
			//ADD PLAYER/SOCKET / SEND HELLOWORLDPACKET
			
			PacketHelloWorld phw;
			phw.header.type = PacketType::HelloWorld;
			ServerPlayer& p = this->_serverw.addPlayer(glm::vec3(0, 0, 0), glm::quat(), tmpsock);
			phw.yourID = p.getID();
			this->_conn->sendToClient((char*)(&phw), sizeof(phw), tmpsock);

			for (size_t l = 0; l < this->_serverw.getPlayers().size(); l++) {
				//SEND CURRENT PLAYERS(ENTITIES) TO NEWLY CONNECTED PLAYER
				if (p.getID() != this->_serverw.getPlayers()[l].getID()) {
					nlohmann::json j;
					Hydra::World::World::getEntity(this->_serverw.getPlayers()[l].getEntity())->serialize(j);
					std::string str = j.dump();
					PacketSpawnEntityServer* pse = (PacketSpawnEntityServer*)malloc(sizeof(PacketSpawnEntity) + str.size() + 1);
					pse->header.type = PacketType::SpawnEntityServer;
					pse->id = this->_serverw.getPlayers()[l].getID();
					pse->len = str.size() + 1;
					memcpy(pse->data, str.c_str(), str.size());
					pse->data[str.size()] = '\0';
					this->_conn->sendToClient((char*)(pse), pse->packetLength(), tmpsock);

					free(pse);
				}
			}

			//SEND NEW ENTITY TO CURRENTLY EXISITNG PLAYERS
			nlohmann::json j;
			Hydra::World::World::getEntity(p.getEntity())->serialize(j);
			std::string str = j.dump();
			PacketSpawnEntityServer* pse = (PacketSpawnEntityServer*)malloc(sizeof(PacketSpawnEntity) + str.size() + 1);
			pse->header.type = PacketType::SpawnEntityServer;
			pse->id = p.getID();
			pse->len = str.size() + 1;
			memcpy(pse->data, str.c_str(), str.size());
			pse->data[str.size()] = '\0';
			this->_conn->sendToAllExceptOne((char*)(pse), pse->packetLength(), tmpsock);
			
			free(pse);
			//this->_serverw.sendPlayers(this->_conn, phw.yourID);
			

			//Update new player about current world?

			//??

			//Create new player for other players

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

HYDRA_NETWORK_API bool NetworkManager::host(IPaddress ip) {
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

HYDRA_NETWORK_API void NetworkManager::quit() { // TODO
	return void();
}

HYDRA_NETWORK_API void startServer(int port) {
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
