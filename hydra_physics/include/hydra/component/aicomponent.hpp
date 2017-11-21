/**
* AIComponent/AI.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <hydra/pathing/behaviour.hpp>

using namespace Hydra::World;
namespace Hydra::Component {
	struct HYDRA_PHYSICS_API AIComponent final : public IComponent<AIComponent, ComponentBits::AI>{
		~AIComponent() final;

		std::shared_ptr<Behaviour> behaviour = nullptr;
		int debugState = 0;
		int damage = 0;
		float radius = 1.0f;

		inline const std::string type() const final { return "AIComponent"; }
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
