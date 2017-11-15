/**
* AIComponent/AI.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once
#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/pathing/behaviour.hpp>
#include <math.h>
#include <SDL2/SDL.h>


using namespace Hydra::World;
namespace Hydra::Component {

	struct HYDRA_PHYSICS_API AIComponent final : public IComponent<AIComponent, ComponentBits::AI>{
		~AIComponent() final;

	std::shared_ptr<Behaviour> behaviour;
	int debugState;
	int damage = 0;
	float radius = 1;

	inline const std::string type() const final { return "AIComponent"; }
	std::shared_ptr<Hydra::World::Entity> getPlayerEntity();
	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};
