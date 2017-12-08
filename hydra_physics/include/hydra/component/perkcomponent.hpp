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
#include <hydra/abilities/abilities.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API PerkComponent final : public IComponent<PerkComponent, ComponentBits::Perk>{
		enum Perk
		{
			PERK_GRENADE,
			PERK_MINE,
			PERK_BULLETSPRAY,
			PERK_DMGUPSIZEUP,
			PERK_SPEEDUP,
			PERK_FASTSHOWLOWDMG,
			PERK_RED,
			AMOUNTOFPERKS
		};
		std::vector<Perk> newPerks = std::vector<Perk>();
		std::vector<Perk> activePerks = std::vector<Perk>();
		std::vector<BaseAbility*> activeAbilities = std::vector<BaseAbility*>();
		size_t activeAbility = 0;
		bool usedAbilityLastFrame = false;

		~PerkComponent() final;
		inline const std::string type() const final { return "PerkComponent"; }
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
