// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Interface to manage the Bullet3 physics library.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/physics/bulletmanager.hpp>

#include <memory>

#include <hydra/component/rigidbodycomponent.hpp>
#include <btBulletDynamicsCommon.h>

class BulletManagerImpl final : public Hydra::Physics::IPhysicsManager {
public:
	BulletManagerImpl() {
		_broadphase = std::make_unique<btDbvtBroadphase>();
		_collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
		_dispatcher = std::make_unique<btCollisionDispatcher>(_collisionConfiguration.get());
		_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
		_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(_dispatcher.get(), _broadphase.get(), _solver.get(), _collisionConfiguration.get());
		_dynamicsWorld->setGravity(btVector3(0, -10, 0));
	}

	~BulletManagerImpl() final {}

	void enable(Hydra::Component::RigidBodyComponent* component) final {
		_dynamicsWorld->addRigidBody(static_cast<btRigidBody*>(component->getRigidBody()));
	}

	void disable(Hydra::Component::RigidBodyComponent* component) final {
		_dynamicsWorld->removeRigidBody(static_cast<btRigidBody*>(component->getRigidBody()));
	}

	void tick(float delta) final {
		_dynamicsWorld->stepSimulation(delta);
	}

private:
	std::unique_ptr<btBroadphaseInterface> _broadphase;
	std::unique_ptr<btDefaultCollisionConfiguration> _collisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> _dispatcher;
	std::unique_ptr<btSequentialImpulseConstraintSolver> _solver;
	std::unique_ptr<btDiscreteDynamicsWorld> _dynamicsWorld;
};

std::unique_ptr<Hydra::Physics::IPhysicsManager> Hydra::Physics::BulletManager::create() {
	return std::unique_ptr<Hydra::Physics::IPhysicsManager>(new BulletManagerImpl());
}
