/**
* Player stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once
#include <hydra/ext/api.hpp>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/abilities/abilityHandler.hpp>
#include <hydra/abilities/buffHandler.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API PlayerComponent final : public IComponent<PlayerComponent, ComponentBits::Player> {
		glm::vec3 weaponOffset = glm::vec3{2, -1.5, -3};
		bool onGround = false;
		bool firstPerson = true;
		bool isDead = false;

		// TODO: Move?!
		AbilityHandler activeAbillies;
		BuffHandler activeBuffs;

		~PlayerComponent() final;

		std::shared_ptr<Hydra::World::Entity> getWeapon();

		inline const std::string type() const final { return "PlayerComponent"; }
		//void upgradeHealth() {
		//	if (activeBuffs.addBuff(BUFF_HEALTHUPGRADE))
		//		activeBuffs.onActivation(maxHealth, health);

		//	if (activeBuffs.addBuff(BUFF_DAMAGEUPGRADE))
		//		activeBuffs.onActivation(maxHealth, health);
		//}

		std::vector<Buffs> getActiveBuffs() { return activeBuffs.getActiveBuffs(); }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
