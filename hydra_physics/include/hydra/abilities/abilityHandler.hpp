#pragma once

#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>

class HYDRA_PHYSICS_API AbilityHandler {
public:
	AbilityHandler();
	~AbilityHandler() {};
	void addAbility(void(AbilityHandler::*newAbility)(Hydra::World::Entity*));
	void useAbility(Hydra::World::Entity* playerEntity);
	void grenadeAbility(Hydra::World::Entity* playerEntity);
	void mineAbility(Hydra::World::Entity* playerEntity);
	void forcePushAbility(Hydra::World::Entity* playerEntity);
private:
	size_t _activeAbility;
	std::vector<void(AbilityHandler::*)(Hydra::World::Entity*)> _abilityList;
	std::vector<int> _cooldownList;
};
