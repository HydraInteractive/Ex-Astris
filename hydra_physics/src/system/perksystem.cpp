#include <hydra/system/perksystem.hpp>

#include <imgui/imgui.h>
#include <fstream>
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
			ImGui::TextWrapped("%s", perkDescriptionText.c_str());
			ImGui::End();
		}
	}

	entities.clear();
}

void PerkSystem::PerkChange(ReadBullet& b, const std::shared_ptr<Hydra::World::Entity>& playerEntity)
{
	auto w = playerEntity->getComponent<PlayerComponent>()->getWeapon()->getComponent<WeaponComponent>();

	glm::vec3 newColour = glm::vec3(b.bulletColor[0], b.bulletColor[1], b.bulletColor[2]);
	glm::vec3 oldColour = glm::vec3(w->color[0], w->color[1], w->color[2]);
	glm::vec3 mixedColour = glm::clamp(glm::normalize((newColour + oldColour)), glm::vec3(0), glm::vec3(1));

	w->meshType = b.meshType;

	if (b.Adder)
	{

		w->damage += b.dmg;
		w->bulletSize += b.bulletSize;
		w->bulletsPerShot = b.bulletPerShot;
		w->maxammo = b.ammoCap;
		w->maxmagammo += b.currentMagAmmo;
		w->ammoPerShot += b.ammoPerShot;
		w->bulletSpread += b.bulletSpread;

		w->color[0] = mixedColour[0];
		w->color[1] = mixedColour[1];
		w->color[2] = mixedColour[2];
		w->glow = b.glow;
		w->glowIntensity = b.glowIntensity;
		perkDescriptionText = b.perkDescription;
	}
	if (b.Multiplier)
	{
		if (b.dmg > 0.0f)
			w->damage *= b.dmg;

		if (b.bulletSize > 0.0f)
			w->bulletSize *= b.bulletSize;

		w->bulletsPerShot *= b.bulletPerShot;
		w->maxammo = b.ammoCap;
		if (b.currentMagAmmo > 0.0f)
			w->maxmagammo *= b.currentMagAmmo;

		if (b.ammoPerShot > 0.0f)
			w->ammoPerShot *= b.ammoPerShot;

		if (b.bulletSpread > 0.0f)
			w->bulletSpread *= b.bulletSpread;



		w->color[0] = mixedColour[0];
		w->color[1] = mixedColour[1];
		w->color[2] = mixedColour[2];
		w->glow = b.glow;
		w->glowIntensity = b.glowIntensity;
		perkDescriptionText = b.perkDescription;
	}
	//Failsafes
	//Damage
	if (w->damage < 0.0f) 
		w->damage = 0.5f;
	//Bullet Size
	if (w->bulletSize < 0.0f)
		w->bulletSize = 0.3f;
	else if (w->bulletSize > 6.0f)
		w->bulletSize = 6.0f;
	//Bullets Per Shot
	if (w->bulletsPerShot < 0)
		w->bulletsPerShot = 1;
	else if (w->bulletsPerShot > 30)
		w->bulletsPerShot = 30;
	//Max Ammo
	if (w->maxammo != 0)
		w->maxammo = 0;
	//else if (w->maxammo > 200)
	//	w->maxammo = 200;
	if (w->maxmagammo < 1)
		w->maxmagammo = 1;
	if (w->bulletSpread < -1)
		w->bulletSpread = -1;
	else if(w->bulletSpread > 1)
		w->bulletSpread = 1;
		
}

void PerkSystem::readFromFile(const char* fileName, ReadBullet &readBullet)
{

	std::string name = fileName;
	std::ifstream file(name, std::ios::binary);
	file.open("assets/perks/" + name + ".PERK");

	file.read(reinterpret_cast<char*>(&readBullet.bulletSize), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.dmg), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.recoil), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.currentMagAmmo), sizeof(int));
	file.read(reinterpret_cast<char*>(&readBullet.bulletSpread), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.roundsPerMinute), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.bulletColor[0]), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.bulletColor[1]), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.bulletColor[2]), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.Adder), sizeof(bool));
	file.read(reinterpret_cast<char*>(&readBullet.Multiplier), sizeof(bool));
	file.read(reinterpret_cast<char*>(&readBullet.glow), sizeof(bool));
	file.read(reinterpret_cast<char*>(&readBullet.glowIntensity), sizeof(float));
	file.read(reinterpret_cast<char*>(&readBullet.meshType), sizeof(int));

	int nrOfChars = 0;
	file.read(reinterpret_cast<char*>(&nrOfChars), sizeof(int));
	char *tempName;
	tempName = new char[nrOfChars];
	file.read(tempName, nrOfChars);
	readBullet.perkDescription.append(tempName, nrOfChars);
	delete[] tempName;


	file.close();
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
	case Hydra::Component::PerkComponent::PERK_RED: {
		ReadBullet b;
		readFromFile("Duck", b);
		PerkChange(b, playerEntity);	
		break;
	}
	case Hydra::Component::PerkComponent::PERK_GREEN: {
		ReadBullet b;
		readFromFile("Test1", b);
		PerkChange(b, playerEntity);
		break;
	}

	default:
		break;
	}
	perkDescriptionTimer = 5;
}
void PerkSystem::registerUI() {}
