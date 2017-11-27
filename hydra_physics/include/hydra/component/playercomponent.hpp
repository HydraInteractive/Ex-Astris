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
#include <hydra/abilities/abilities.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API PlayerComponent final : public IComponent<PlayerComponent, ComponentBits::Player> {
		glm::vec3 weaponOffset = glm::vec3{0.2f, -0.15f, -0.3f};
		bool onGround = true;
		bool firstPerson = true;
		bool isDead = false;

		~PlayerComponent() final;

		std::shared_ptr<Hydra::World::Entity> getWeapon();

		inline const std::string type() const final { return "PlayerComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
