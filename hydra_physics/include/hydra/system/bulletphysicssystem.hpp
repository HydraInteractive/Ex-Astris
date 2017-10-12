/**
 * Interface to manage the Bullet3 physics library.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once

#include <hydra/world/world.hpp>
#include <btBulletDynamicsCommon.h>

namespace Hydra::System {
	class BulletPhysicsSystem final : public Hydra::World::ISystem {
	public:
		enum class CollisionShape {
			Box,
			StaticPlane,
			Sphere,
			//TriangleMesh, // btBvhTriangleMesh
			CapsuleX,
			CapsuleY, // btCapsuleShape
			CapsuleZ,
			CylinderX,
			CylinderY, // btCylinderShape
			CylinderZ
		};
		BulletPhysicsSystem();
		~BulletPhysicsSystem() final;

		void enable(Hydra::Component::RigidBodyComponent* component);

		void disable(Hydra::Component::RigidBodyComponent* component);

		void tick(float delta) final;

		inline const std::string type() const final { return "BulletPhysicsSystem"; }
		void registerUI() final;

	private:
		std::unique_ptr<btBroadphaseInterface> _broadphase;
		std::unique_ptr<btDefaultCollisionConfiguration> _collisionConfiguration;
		std::unique_ptr<btCollisionDispatcher> _dispatcher;
		std::unique_ptr<btSequentialImpulseConstraintSolver> _solver;
		std::unique_ptr<btDiscreteDynamicsWorld> _dynamicsWorld;
	};
}
