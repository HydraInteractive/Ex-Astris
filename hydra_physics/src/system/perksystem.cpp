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
			&& perks->activeAbilities[perks->activeAbility]->cooldown <= 0) {
			perks->activeAbilities[perks->activeAbility]->useAbility(entities[i]);
			perks->activeAbilities[perks->activeAbility]->cooldown = 1;
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
		
		perkDescriptionTimer -= delta;
		if (perkDescriptionTimer > 0){
			ImGui::SetNextWindowPos(ImVec2(50, 720-100-50));
			ImGui::SetNextWindowSize(ImVec2(300, 100));
			ImGui::Begin("Perk Description", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove);
			ImGui::TextWrapped(perkDescriptionText.c_str());
			ImGui::End();
		}
	}

	entities.clear();
}
void PerkSystem::onPickUp(Hydra::Component::PerkComponent::Perk newPerk, const std::shared_ptr<Hydra::World::Entity>& playerEntity) {
	auto perk = playerEntity->getComponent<PerkComponent>();
	perk->activePerks.push_back(newPerk);

	switch (newPerk){
	case Hydra::Component::PerkComponent::PERK_GRENADE: {
		perk->activeAbilities.push_back(new GrenadeAbility());
		perkDescriptionText = "One big grenade perk boio";
		break;
	}
	case Hydra::Component::PerkComponent::PERK_MINE: {
		perk->activeAbilities.push_back(new MineAbility());
		perkDescriptionText = "One slidey mine thing";
		break;
	}
	case Hydra::Component::PerkComponent::PERK_BULLETSPRAY: {
		perk->activeAbilities.push_back(new BulletSprayAbillity());
		perkDescriptionText = "Press F to spray to victory";
		break;
	}
	case Hydra::Component::PerkComponent::PERK_DMGUPSIZEUP: {
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->bulletSize *= 2;
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->damage*= 2;
		perkDescriptionText = "DMG ++++++++++ BIG BOI BULLET";
		break;
	}
	case Hydra::Component::PerkComponent::PERK_SPEEDUP: {
		auto weapon = playerEntity->getComponent<MovementComponent>()->movementSpeed *= 2;
		perkDescriptionText = "IM FAST AS FUCK BOI";
		break;
	}
	case Hydra::Component::PerkComponent::PERK_FASTSHOWLOWDMG: {
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->fireRateRPM *= 3;
		playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>()->damage /= 2.5;
		perkDescriptionText = "SHIIIIIET SHOOOTY FAST AND RECOILI UP";
		break;
	}
															
	default:
		break;
	}
	perkDescriptionTimer = 5;
}
void PerkSystem::registerUI() {}
