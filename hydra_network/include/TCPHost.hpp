#pragma once
#include <../hydra_network/include/TCPConnection.hpp>

HYDRA_API class TCPHost : public TCPConnection {
private:
	std::vector<TCPsocket> _clients;
	TCPsocket _socket;
public:
	HYDRA_API TCPHost();
	HYDRA_API ~TCPHost();
	HYDRA_API bool initiate(IPaddress ip, char* s = "");
	HYDRA_API void close();
	HYDRA_API TCPsocket checkForClient();

	HYDRA_API void update();

	HYDRA_API void sendToClients(char* data);
	HYDRA_API void sendEntites(std::vector<std::shared_ptr<Hydra::World::IEntity>> list);
	//void sendEntity(std::shared_ptr<Hydra::World::IEntity> entity);
};