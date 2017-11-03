#pragma once
#include <hydra/ext/api.hpp>
#include <Packets.h>
#include <TCPClient.hpp>
#include <hydra/world/world.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>


class HYDRA_NETWORK_API NetPlayer {
private:
	int64_t _id;
	int64_t _player;
	bool _isDead;
public:
	NetPlayer();
	~NetPlayer();

	inline void setIsDead(bool isDead) { this->_isDead = isDead; }
	//HYDRA_NETWORK_API inline void setPlayer(std::shared_ptr<Hydra::World::IEntity> p) { this->_player = p; }
	inline int64_t getID() { return this->_id; }
	inline void setID(EntityID id) { this->_id = id; }
	void addPlayer(EntityID ent);
	TransformInfo getPlayerInfo();
};
