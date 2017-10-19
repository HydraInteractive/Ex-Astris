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
#include <SDL2/SDL_mixer.h>
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

enum Keys {
	H, F, COUNT
};

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API PlayerComponent final : public IComponent<PlayerComponent, ComponentBits::Player> {
		glm::vec3 position = glm::vec3(0, 0, 0); //TODO: Remove
		glm::vec3 velocity = glm::vec3{0, 0, 0};// Same
		glm::vec3 acceleration = glm::vec3{0, 0, 0}; // Same
		glm::vec3 weaponOffset = glm::vec3{2, -1.5, -3};
		float movementSpeed = 20.0f;
		bool onGround = false;
		bool firstPerson = true;
		int maxHealth = 100; // Move to LifeComponent
		int health = 100; // SAME
		bool isDead = false;
		bool prevKBFrameState[Keys::COUNT] = { false };

		// TODO: Move?!
		AbilityHandler activeAbillies;
		BuffHandler activeBuffs;

		~PlayerComponent() final;

		std::shared_ptr<Hydra::World::Entity> getWeapon();

		inline const std::string type() const final { return "PlayerComponent"; }
		void upgradeHealth() {
			if (activeBuffs.addBuff(BUFF_HEALTHUPGRADE))
				activeBuffs.onActivation(maxHealth, health);

			if (activeBuffs.addBuff(BUFF_DAMAGEUPGRADE))
				activeBuffs.onActivation(maxHealth, health);
		}

		std::vector<Buffs> getActiveBuffs() { return activeBuffs.getActiveBuffs(); }
		void applyDamage(int damage) { health -= damage; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
