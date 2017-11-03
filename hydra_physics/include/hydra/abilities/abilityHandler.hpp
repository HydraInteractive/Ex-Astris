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
	void addAbility(void(AbilityHandler::*test)(Hydra::World::Entity*, glm::vec3, glm::vec3));
	void useAbility(Hydra::World::Entity* abilitiesEntity, glm::vec3 position, glm::vec3 direction);
	void grenadeAbility(Hydra::World::Entity* abilitiesEntity, glm::vec3 position, glm::vec3 direction);
	void mineAbility(Hydra::World::Entity* abilitiesEntity, glm::vec3 position, glm::vec3 direction);
	void forcePushAbility(Hydra::World::Entity* abilitiesEntity, glm::vec3 position, glm::vec3 direction);
private:
	size_t _activeAbility;
	std::vector<void(AbilityHandler::*)(Hydra::World::Entity*, glm::vec3, glm::vec3)> _abilityList;
	std::vector<int> _cooldownList;
};
