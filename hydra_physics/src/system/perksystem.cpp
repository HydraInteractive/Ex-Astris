#include <hydra/system/perksystem.hpp>

#include <imgui/imgui.h>

#include <hydra/ext/openmp.hpp>
#include <hydra/engine.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/perkcomponent.hpp>
#include <hydra/component/movementcomponent.hpp>

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
		while (!perks->newPerks.empty()){
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
		for (size_t j = 0; j < perks->activeAbilities.size(); j++) {
			perks->activeAbilities[j]->activeTimer -= delta;
			perks->activeAbilities[j]->timeSinceLastTick += delta;

			if (perks->activeAbilities[j]->activeTimer >= 0 && perks->activeAbilities[j]->tickFreq <= perks->activeAbilities[j]->timeSinceLastTick) {
				perks->activeAbilities[j]->tick(delta, entities[i]);
				perks->activeAbilities[j]->timeSinceLastTick = 0;
			}
			if (perks->activeAbilities[j]->activeTimer < 0 && perks->activeAbilities[j]->afterLastTick){
				perks->activeAbilities[j]->doneTick(delta,entities[i]);
				perks->activeAbilities[j]->afterLastTick = false;
			}
		}

		perks->usedAbilityLastFrame = keysArray[SDL_SCANCODE_F];
	}

	entities.clear();
}
void PerkSystem::onPickUp(Hydra::Component::PerkComponent::Perk newPerk, const std::shared_ptr<Hydra::World::Entity>& playerEntity) {
	auto perk = playerEntity->getComponent<PerkComponent>();
	perk->activePerks.push_back(newPerk);

	switch (newPerk){
	/*case Hydra::Component::PerkComponent::PERK_MAGNETICBULLETS:	{
		auto weapon = playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>();
		weapon->bulletType = BulletComponent::BulletType::magnetic;
		break;
	}*/
	/*case Hydra::Component::PerkComponent::PERK_HOMINGBULLETS: {
		auto weapon = playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>();
		weapon->bulletType = BulletComponent::BulletType::homing;
		break;
	}*/
	case Hydra::Component::PerkComponent::PERK_GRENADE: {
		perk->activeAbilities.push_back(new GrenadeAbility());
		break;
	}
	case Hydra::Component::PerkComponent::PERK_MINE: {
		perk->activeAbilities.push_back(new MineAbility());
		break;
	}
	/*case Hydra::Component::PerkComponent::PERK_FORCEPUSH: {
		perk->activeAbilities.push_back(new forcePushAbility());
		break;
	}*/
	case Hydra::Component::PerkComponent::PERK_BULLETSPRAY: {
		perk->activeAbilities.push_back(new BulletSprayAbillity());
		break;
	}
	case Hydra::Component::PerkComponent::PERK_DMGUPSIZEUP: {
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->bulletSize *= 2;
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->damage*= 2;

		break;
	}
	case Hydra::Component::PerkComponent::PERK_SPEEDUP: {
		auto weapon = playerEntity->getComponent<MovementComponent>()->movementSpeed *= 2;
		break;
	}
	case Hydra::Component::PerkComponent::PERK_FASTSHOWLOWDMG: {
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->fireRateRPM *= 3;
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->damage /= 2.5;
		break;
	}
															
	default:
		break;
	}
}
void PerkSystem::registerUI() {}
