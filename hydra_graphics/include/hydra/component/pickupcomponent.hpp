/**
* Pickup stuff
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

using namespace Hydra::World;

enum PickUpType {
	HealthPickUp,
	RandomPerkPickUp
};

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API PickUpComponent final : public IComponent<PickUpComponent, ComponentBits::PickUp>{

	PickUpType pickUpType;

	~PickUpComponent() final;

	inline const std::string type() const final { return "PickUpComponent"; }

	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};
