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
			//PERK_GRENADE,
			//PERK_MINE,
			//PERK_BULLETSPRAY,
			PERK_DMGUPSIZEUP,
			PERK_SPEEDUP,
			PERK_FASTSHOWLOWDMG,
			PERK_BANANA1,
			PERK_DUCK,
			HP_DOWN1,
			HP_UP1,
			HP_UP2,
			HP_UP3,
			PERK_HPDMGSTAR1,
			PERK_POSEIDONSCURSE,
			PERK_AMMOCAPDOWNDAMAGEUP1,
			PERK_PURPLETRIDENT,
			PERK_SNIPINGTRIDENT,
			PERK_DMGUP1,
			PERK_DMGDOWN1,
			PERK_BULLETSEVERYWHERE1,
			PERK_BULLETSEVERYWHERE2,
			PERK_CALMDOWN,
			PERK_DMGUPSPREADDOWN1,
			PERK_EXTRASHOT1,
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
