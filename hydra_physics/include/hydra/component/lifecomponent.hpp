/**
* Get a life stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <hydra/world/world.hpp>


using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API LifeComponent final : public IComponent<LifeComponent, ComponentBits::Life>{
	int maxHP = 100;
	int health = 100;

	~LifeComponent() final;

	void applyDamage(int damage);
	bool statusCheck();

	inline const std::string type() const final { return "LifeComponent"; }

	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};
