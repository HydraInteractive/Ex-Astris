#include <hydra/system/perksystem.hpp>

#include <imgui/imgui.h>

#include <hydra/ext/openmp.hpp>
#include <hydra/engine.hpp>

using namespace Hydra::System;
using namespace Hydra::Component;

using world = Hydra::World::World;

PerkSystem::PerkSystem() {}
PerkSystem::~PerkSystem() {}

void PerkSystem::tick(float delta) {
	const Uint8* keysArray = SDL_GetKeyboardState(nullptr);

	world::getEntitiesWithComponents<PlayerComponent, PerkComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto perks = entities[i]->getComponent<PerkComponent>();

		//Adding new perks
		for (size_t i = 0; i < perks->newPerks.size(); i++){
			onPickUp(perks->newPerks.back(), entities[i]);
			perks->newPerks.pop_back(); 
		}

		//Use active ability
		if (keysArray[SDL_SCANCODE_F] 
			&& !perks->usedAbilityLastFrame 
			&& !perks->activeAbilities.empty()
			&& perks->activeAbilities[perks->activeAbility]->cooldown == 0) {
			perks->activeAbilities[perks->activeAbility]->useAbility(entities[i]);
			if (++perks->activeAbility >= perks->activeAbilities.size())
				perks->activeAbility = 0;
		}

		//Active abilities tick
		for (size_t i = 0; i < perks->activeAbilities.size(); i++) {
			perks->activeAbilities[i]->activeTimer -= delta;
			perks->activeAbilities[i]->timeSinceLastTick += delta;

			if (perks->activeAbilities[i]->activeTimer >= 0 && perks->activeAbilities[i]->tickFreq <= perks->activeAbilities[i]->timeSinceLastTick) {
				perks->activeAbilities[i]->tick(delta, entities[i]);
				perks->activeAbilities[i]->timeSinceLastTick = 0;
			}
		}

		perks->usedAbilityLastFrame = keysArray[SDL_SCANCODE_F];
	}
}
void PerkSystem::onPickUp(Perk newPerk, const std::shared_ptr<Hydra::World::Entity>& playerEntity) {
	switch (newPerk){
	case PERK_MAGNETICBULLETS:	{
		auto weapon = playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>();
		weapon->bulletType = BULLETTYPE_MAGNETIC;
		break;
	}
	case PERK_HOMINGBULLETS: {
		auto weapon = playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>();
		weapon->bulletType = BULLETTYPE_HOMING;
		break;
	}
	case PERK_GRENADE: {
		auto perk = playerEntity->getComponent<PerkComponent>();
		perk->activeAbilities.push_back(new GrenadeAbility());
		break;
	}
	case PERK_MINE: {
		auto perk = playerEntity->getComponent<PerkComponent>();
		perk->activeAbilities.push_back(new MineAbility());
		break;
	}
	case PERK_FORCEPUSH: {
		auto perk = playerEntity->getComponent<PerkComponent>();
		perk->activeAbilities.push_back(new forcePushAbility());
		break;
	}
	case PERK_BULLETSPRAY: {
		auto perk = playerEntity->getComponent<PerkComponent>();
		perk->activeAbilities.push_back(new BulletSprayAbillity());
		break;
	}
	case PERK_DASH: {
		auto perk = playerEntity->getComponent<PerkComponent>();
		perk->activeAbilities.push_back(new DashAbility());
		break;
	}
	default:
		break;
	}
}
void PerkSystem::registerUI() {}
