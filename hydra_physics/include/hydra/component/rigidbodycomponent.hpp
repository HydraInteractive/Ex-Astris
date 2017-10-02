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

#include <hydra/component/transformcomponent.hpp>

namespace Hydra::Component {
	class HYDRA_API RigidBodyComponent final : public Hydra::World::IComponent {
	public:
		struct Data;
		RigidBodyComponent(IEntity* entity);
		RigidBodyComponent(IEntity* entity, std::unique_ptr<Data> data);
		~RigidBodyComponent() final;

		static std::unique_ptr<RigidBodyComponent> createBox(IEntity* entity, const glm::vec3& halfExtents, float mass = 0, float linearDamping = 0, float angularDamping = 0, float friction = 0, float rollingFriction = 0);
		static std::unique_ptr<RigidBodyComponent> createStaticPlane(IEntity* entity, const glm::vec3& planeNormal, float planeConstant, float mass = 0, float linearDamping = 0, float angularDamping = 0, float friction = 0, float rollingFriction = 0);

		void* getRigidBody();

		void tick(Hydra::World::TickAction action, float delta) final;
		inline Hydra::World::TickAction wantTick() const final { return Hydra::World::TickAction::none; }

		inline const std::string type() const final { return "RigidBodyComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	private:
		std::unique_ptr<Data> _data;
	};
};
