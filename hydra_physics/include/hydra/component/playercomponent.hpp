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
	struct HYDRA_API PlayerComponent final : public IComponent<PlayerComponent, ComponentBits::Player> {
		glm::vec3 _position = glm::vec3(0, 2, 20);
		glm::vec3 _weaponOffset = glm::vec3{2, -1.5, -3};
		glm::vec3 _velocity = glm::vec3{0, 0, 0};
		glm::vec3 _acceleration = glm::vec3{0, 0, 0};
		float _movementSpeed = 20.0f;
		bool _onGround = false;
		bool _firstPerson = true;
		float _timer = 0.5;
		int _maxHealth = 100;
		int _health = 100;
		bool _dead = false;

		AbilityHandler _activeAbillies;
		BuffHandler _activeBuffs;

		~PlayerComponent() final;

		std::shared_ptr<Hydra::World::IEntity> getWeapon();

		inline const std::string type() const final { return "PlayerComponent"; }
		void upgradeHealth(){
			if (_activeBuffs.addBuff(BUFF_HEALTHUPGRADE)) {
				_activeBuffs.onActivation(_maxHealth, _health);
			}
			if (_activeBuffs.addBuff(BUFF_DAMAGEUPGRADE)) {
				_activeBuffs.onActivation(_maxHealth, _health);
			}
		}
		void applyDamage(float delta, int damage);

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
