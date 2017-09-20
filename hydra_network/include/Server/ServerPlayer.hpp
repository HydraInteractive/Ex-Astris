#pragma once
#include <hydra/world/world.hpp>
#include <NetPlayer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>



class ServerPlayer {
private:
	int64_t _id;
	TCPsocket _sock;
	std::shared_ptr<Hydra::World::IEntity> _entity;
public:
	ServerPlayer();
	~ServerPlayer();
	HYDRA_API inline void initialize(int64_t id, TCPsocket sock, std::shared_ptr<Hydra::World::IEntity> player) { this->_id = id; this->_sock = sock; this->_entity = player; }
	HYDRA_API inline int64_t getID() { return this->_id; }
	HYDRA_API inline TCPsocket getSocket() { return this->_sock; }
	HYDRA_API inline std::shared_ptr<Hydra::World::IEntity> getEntity() { return this->_entity; }
};
