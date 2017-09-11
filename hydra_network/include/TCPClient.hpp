#pragma once

#include <../hydra_network/include/TCPConnection.hpp>
HYDRA_API class TCPClient : public TCPConnection {
private:
	TCPsocket _socket;
public:
	HYDRA_API TCPClient();
	HYDRA_API ~TCPClient();
	HYDRA_API bool initiate(IPaddress ip, char* c);
	HYDRA_API void close();
	HYDRA_API void update();

	HYDRA_API void receivePacket();
	HYDRA_API void sendEntity(std::shared_ptr<Hydra::World::IEntity> entity);
};