#include <NetPlayer.hpp>
#include <hydra/component/transformcomponent.hpp>

NetPlayer::NetPlayer() {
	_isDead = true;
}

NetPlayer::~NetPlayer() {
}

HYDRA_NETWORK_API void NetPlayer::addPlayer(EntityID ent) {
	this->_player = ent;
	this->_isDead = false;
}

HYDRA_NETWORK_API TransformInfo NetPlayer::getPlayerInfo() {
	if (!this->_isDead) {
		Hydra::Component::TransformComponent* tc = Hydra::World::World::getEntity(this->_player)->getComponent<Hydra::Component::TransformComponent>().get();
		TransformInfo pi;
		pi.position = tc->position;
		pi.rot = tc->rotation;
		pi.scale = tc->scale;
		return pi;
	}
	return TransformInfo();
}
