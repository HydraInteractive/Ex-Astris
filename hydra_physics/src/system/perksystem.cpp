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
	world::getEntitiesWithComponents<PlayerComponent, PerkComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto perks = entities[i]->getComponent<PerkComponent>();

		for (size_t i = 0; i < perks->newPerks.size(); i++){
			onPickUp(perks->newPerks.back(), entities[i].get());
			perks->newPerks.pop_back();
		}

		for (size_t i = 0; i < perks->activePerks.size(); i++)
		{
			onTick(perks->activePerks[i]);
		}
	}
}
void PerkSystem::onPickUp(Perk newPerk, Entity* playerEntity) {
	switch (newPerk){
	case PERK_MAGNETICBULLEETS:	{
		
	}
		break;
	case PERK_HOMINGBULLETS: {

	}
		break;
	case PERK_GRENADE: {
		auto player = playerEntity->getComponent<PlayerComponent>();
		player->activeAbillies.addAbility(&AbilityHandler::grenadeAbility);
	}
		break;
	case PERK_MINE: {
		auto player = playerEntity->getComponent<PlayerComponent>();
		player->activeAbillies.addAbility(&AbilityHandler::mineAbility);
	}
		break;
	default:
		break;
	}
}
void PerkSystem::onTick(Perk activePerk) {
	switch (activePerk)
	{
	case PERK_MAGNETICBULLEETS:
		break;
	case PERK_HOMINGBULLETS:
		break;
	case PERK_GRENADE:
		break;
	case PERK_MINE:
		break;
	default:
		break;
	}
}
void PerkSystem::registerUI() {}
