#pragma once
#include <hydra/world/world.hpp>
#include <NetPlayer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <hydra/ext/api.hpp>


class ServerPlayer {
private:
	int64_t _id;
	TCPsocket _sock;
	EntityID _entity;
public:
	ServerPlayer();
	~ServerPlayer();
	HYDRA_NETWORK_API inline void initialize(EntityID id, TCPsocket sock, int64_t player) { this->_id = id; this->_sock = sock; this->_entity = player; }
	HYDRA_NETWORK_API inline int64_t getID() { return this->_id; }
	HYDRA_NETWORK_API inline TCPsocket getSocket() { return this->_sock; }
	HYDRA_NETWORK_API inline EntityID getEntity() { return this->_entity; }
};
