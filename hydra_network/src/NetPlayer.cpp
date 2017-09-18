#include "..\include\NetPlayer.hpp"
#include <hydra/component/transformcomponent.hpp>

NetPlayer::NetPlayer() {
	_isDead = true;
}

NetPlayer::~NetPlayer() {
}

HYDRA_API void NetPlayer::addPlayer(std::shared_ptr<Hydra::World::IEntity> ent) {
	this->_player = ent;
	this->_isDead = false;
}

HYDRA_API TransformInfo NetPlayer::getPlayerInfo() {
	if (!this->_isDead) {
		Hydra::Component::TransformComponent* tc = this->_player->getComponent<Hydra::Component::TransformComponent>();
		TransformInfo pi;
		pi.position = tc->getPosition();
		pi.rot = tc->getRotation();
		pi.scale = tc->getScale();
		return pi;
	}
	return TransformInfo();
}
