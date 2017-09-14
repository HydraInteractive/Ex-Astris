/**
 * Description of the component.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */

#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API NAMEComponent final : public IComponent {
	public:
		NAMEComponent(IEntity* entity); // This one must exist
		NAMEComponent(IEntity* entity, int a, bool b, float c);
		~NAMEComponent() final;

		void tick(TickAction action) final;
		// If you want to add more than one TickAction, combine them with '|' (The bitwise or operator) 
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "NAMEComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	private:
		// Set default values here, on in the constructor
		int _a = 0;
		bool _b = false;
		float _c; // This default value is set in the constructor
	};
};
