#pragma once
/**
* Mine stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/


#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API MineComponent final : public IComponent<MineComponent, ComponentBits::Mine> {
		glm::vec3 direction = glm::vec3{0, 0, 0};
		float velocity = 02;
		float fallingVelocity = 20;
		float timer = 5;

		~MineComponent() final;

		inline const std::string type() const final { return "GrenadeComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
