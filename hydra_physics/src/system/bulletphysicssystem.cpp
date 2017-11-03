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

struct BulletPhysicsSystem::Data {
	std::unique_ptr<btDefaultCollisionConfiguration> config;
	std::unique_ptr<btCollisionDispatcher> dispatcher;
	std::unique_ptr<btBroadphaseInterface> broadphase;
	std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
	std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
};

BulletPhysicsSystem::BulletPhysicsSystem() {
	_data = new Data;
	_data->config = std::make_unique<btDefaultCollisionConfiguration>();
	_data->dispatcher = std::make_unique<btCollisionDispatcher>(_data->config.get());
	_data->broadphase = std::make_unique<btDbvtBroadphase>();
	_data->solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	_data->dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(_data->dispatcher.get(), _data->broadphase.get(), _data->solver.get(), _data->config.get());
	_data->dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

BulletPhysicsSystem::~BulletPhysicsSystem() { delete _data; }

void BulletPhysicsSystem::enable(Hydra::Component::RigidBodyComponent* component) {
	component->_handler = this;
	_data->dynamicsWorld->addRigidBody(static_cast<btRigidBody*>(component->getRigidBody()));
}

void BulletPhysicsSystem::disable(Hydra::Component::RigidBodyComponent* component) {
	_data->dynamicsWorld->removeRigidBody(static_cast<btRigidBody*>(component->getRigidBody()));
	component->_handler = nullptr;
}

void BulletPhysicsSystem::tick(float delta) {
	_data->dynamicsWorld->stepSimulation(delta);
}

void BulletPhysicsSystem::registerUI() {}
