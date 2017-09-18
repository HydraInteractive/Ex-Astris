#pragma once
#include <../hydra_network/include/Packets.h>
#include <../hydra_network/include/TCPClient.hpp>
#include <hydra/world/world.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>


class NetPlayer {
private:
	int64_t _id;
	std::shared_ptr<Hydra::World::IEntity> _player;
	bool _isDead;
public:
	HYDRA_API NetPlayer();
	HYDRA_API ~NetPlayer();

	HYDRA_API inline void setIsDead(bool isDead) { this->_isDead = isDead; }
	//HYDRA_API inline void setPlayer(std::shared_ptr<Hydra::World::IEntity> p) { this->_player = p; }
	HYDRA_API inline int64_t getID() { return this->_id; }
	HYDRA_API inline void setID(int64_t id) { this->_id = id; }
	HYDRA_API void addPlayer(std::shared_ptr<Hydra::World::IEntity> ent);
	HYDRA_API TransformInfo getPlayerInfo();
};
