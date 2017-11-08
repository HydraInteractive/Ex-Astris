
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
#include <hydra/system/bulletphysicssystem.hpp>

#include <hydra/component/transformcomponent.hpp>

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API RigidBodyComponent final : public Hydra::World::IComponent<RigidBodyComponent, ComponentBits::RigidBody> {
		friend class Hydra::System::BulletPhysicsSystem;
		~RigidBodyComponent() final;

#define DEFAULT_PARAMS Hydra::System::BulletPhysicsSystem::CollisionTypes collType = Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_NOTHING, float mass = 0, float linearDamping = 0, float angularDamping = 0, float friction = 0, float rollingFriction = 0
		void createBox(const glm::vec3& halfExtents, DEFAULT_PARAMS);
		void createStaticPlane(const glm::vec3& planeNormal, float planeConstant, DEFAULT_PARAMS);
		void createSphere(float radius, DEFAULT_PARAMS);
		//void createTriangleMesh(btStridingMeshInterface *meshInterface, bool useQuantizedAabbCompression, bool buildBvh=true, DEFAULT_PARAMS);
		void createCapsuleX(float radius, float height, DEFAULT_PARAMS);
		void createCapsuleY(float radius, float height, DEFAULT_PARAMS);
		void createCapsuleZ(float radius, float height, DEFAULT_PARAMS);
		void createCylinderX(const glm::vec3& halfExtents, DEFAULT_PARAMS);
		void createCylinderY(const glm::vec3& halfExtents, DEFAULT_PARAMS);
		void createCylinderZ(const glm::vec3& halfExtents, DEFAULT_PARAMS);
#undef DEFAULT_PARAMS

		void setActivationState(const int newState);
		void* getRigidBody();

		inline const std::string type() const final { return "RigidBodyComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	private:
		Hydra::System::BulletPhysicsSystem::CollisionTypes doa;
		struct Data;
		Data* _data;
		Hydra::System::BulletPhysicsSystem* _handler;
	};
};
