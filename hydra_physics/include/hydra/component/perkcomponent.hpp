/**
* Perk stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>

using namespace Hydra::World;

enum Perk
{
	PERK_MAGNETICBULLETS,
	PERK_HOMINGBULLETS,
	PERK_GRENADE,
	PERK_MINE,
	PERK_FORCEPUSH
};

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API PerkComponent final : public IComponent<PerkComponent, ComponentBits::Perk>{

		std::vector<Perk> newPerks;
		std::vector<Perk> activePerks;

		~PerkComponent() final;
		inline const std::string type() const final { return "PerkComponent"; }
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
