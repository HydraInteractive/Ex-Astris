#pragma once
/**
* Grenade stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <hydra/world/world.hpp>
#include <hydra/component/perkcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API GrenadeComponent final : public IComponent<GrenadeComponent, ComponentBits::Grenade> {
		Perk perkType = PERK_GRENADE;
		bool isExploding = false;
		float detonateTimer = 4;

		~GrenadeComponent() final;

		inline const std::string type() const final { return "GrenadeComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
