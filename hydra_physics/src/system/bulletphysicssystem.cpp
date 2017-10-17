// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Interface to manage the Bullet3 physics library.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/system/bulletphysicssystem.hpp>

#include <memory>

#include <hydra/component/rigidbodycomponent.hpp>
#include <btBulletDynamicsCommon.h>

using namespace Hydra::System;

BulletPhysicsSystem::BulletPhysicsSystem() {
	_broadphase = std::make_unique<btDbvtBroadphase>();
	_collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	_dispatcher = std::make_unique<btCollisionDispatcher>(_collisionConfiguration.get());
	_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(_dispatcher.get(), _broadphase.get(), _solver.get(), _collisionConfiguration.get());
	_dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

BulletPhysicsSystem::~BulletPhysicsSystem() {}

void BulletPhysicsSystem::enable(Hydra::Component::RigidBodyComponent* component) {
	_dynamicsWorld->addRigidBody(static_cast<btRigidBody*>(component->getRigidBody()));
}

void BulletPhysicsSystem::disable(Hydra::Component::RigidBodyComponent* component) {
	_dynamicsWorld->removeRigidBody(static_cast<btRigidBody*>(component->getRigidBody()));
}

void BulletPhysicsSystem::tick(float delta) {
	_dynamicsWorld->stepSimulation(delta);
}

void BulletPhysicsSystem::registerUI() {}
