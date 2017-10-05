#include <hydra/abilities/buffHandler.hpp>

BuffHandler::BuffHandler(){

}

BuffHandler::~BuffHandler(){

}

std::vector<Buffs> BuffHandler::getActiveBuffs() {
	return _activeBuffs;
}
void BuffHandler::getActiveDebuffs() {
	for (size_t i = 0; i < _activeDebuffs.size(); i++) {
		switch (_activeDebuffs[i]) {
		case DEBUFF_POISON:
			break;
		case DEBUFF_BURNING:
			break;
		default:
			break;
		}
	}
}

bool BuffHandler::addBuff(Buffs newBuff) {
	bool buffCanBeAdded = true;
	for (int i = 0; i < _activeBuffs.size(); i++){
		if (_activeBuffs[i] == newBuff) {
			buffCanBeAdded = false;
		}
	}
	if (buffCanBeAdded)
	{
		_activeBuffs.push_back(newBuff);
		_activeBuffsTimer.push_back(SDL_GetTicks());
	}
	return buffCanBeAdded;
}

void BuffHandler::onActivation(int &maxHealth, int &health) {
	for (size_t i = _lastPickedUpBuff; i < _activeBuffs.size(); i++){
		switch (_activeBuffs[i]) {
		case BUFF_HEALTHUPGRADE:
			int lastMax = maxHealth;
			maxHealth *= 1.25;
			health += maxHealth - lastMax;
		}
	}
	_lastPickedUpBuff++;
}

void BuffHandler::onTick(int &maxHealth, int &health) {
	for (size_t i = 0; i < _activeBuffs.size(); i++) {
		switch (_activeBuffs[i]) {
		case BUFF_HEALING:
			if (SDL_GetTicks() > _activeBuffsTimer[i] + 3000) {
				health += 10;
				if (health > maxHealth) { health = maxHealth; }
				_activeBuffsTimer[i] = SDL_GetTicks();
			}
			break;
		}
	}
}

void BuffHandler::onAttack(float &bulletVelocity) {
	for (size_t i = 0; i < _activeBuffs.size(); i++){
		switch (_activeBuffs[i]){
		case BUFF_BULLETVELOCITY:
			bulletVelocity *= 4.0f;
			break;
		}
	}
}