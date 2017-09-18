#pragma once
#include <hydra/world/world.hpp>
#include <../hydra_network/include/NetPlayer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class ServerPlayer {
private:
	int64_t _id;
	TCPsocket _sock;
public:
	ServerPlayer();
	~ServerPlayer();
	HYDRA_API inline void initialize(int64_t id, TCPsocket sock) { this->_id = id; this->_sock = sock; }
	HYDRA_API inline int64_t getID() { return this->_id; }
	HYDRA_API inline TCPsocket getSocket() { return this->_sock; }
};
