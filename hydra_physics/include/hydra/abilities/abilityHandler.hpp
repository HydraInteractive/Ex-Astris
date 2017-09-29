#pragma once

#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>

class AbilityHandler
{
public:
	AbilityHandler();
	~AbilityHandler() {};
	void useAbility(Hydra::World::IEntity* abilitiesEntity, glm::vec3 position, glm::vec3 direction);
	void grenadeAbility(Hydra::World::IEntity* abilitiesEntity, glm::vec3 position, glm::vec3 direction);
	void mineAbility(Hydra::World::IEntity* abilitiesEntity, glm::vec3 position, glm::vec3 direction);
private:
	int _activeAbility;
	std::vector<void(AbilityHandler::*)(Hydra::World::IEntity*, glm::vec3, glm::vec3)> _abilityList;
	std::vector<int> _cooldownList;
};