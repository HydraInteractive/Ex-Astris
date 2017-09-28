/**
 * A component that enables the entity to be updated with physics.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <hydra/physics/physicsmanager.hpp>

namespace Hydra::Component {
	class HYDRA_API RigidBodyComponent final : public IComponent {
	public:
		RigidBodyComponent(IEntity* entity);
		~RigidBodyComponent() final;

		void tick(TickAction action, float delta) final;
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "RigidBodyComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	private:
		struct Data;
		std::unique_ptr<Data> _data;
	};
}
