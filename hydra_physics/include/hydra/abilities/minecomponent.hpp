#pragma once
/**
* Mine stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API MineComponent final : public IComponent<MineComponent, ComponentBits::Mine> {
		bool isExploding = false;
		float detonateTimer = 20.0;

		~MineComponent() final;

		inline const std::string type() const final { return "MineComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
