#include <hydra/system/pickupsystem.hpp>
#include <hydra/ext/openmp.hpp>
#include <hydra/engine.hpp>
#include <imgui/imgui.h>
#include <btBulletDynamicsCommon.h>

#include <hydra/component/pickupcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>

using namespace Hydra::System;
using namespace Hydra::Component;

using world = Hydra::World::World;

PickUpSystem::PickUpSystem() {}
PickUpSystem::~PickUpSystem() {}

void PickUpSystem::tick(float delta) {
	world::getEntitiesWithComponents<PickUpComponent, RigidBodyComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto rgbc = entities[i]->getComponent<RigidBodyComponent>();
		auto rbc = static_cast<btRigidBody*>(entities[i]->getComponent<RigidBodyComponent>()->getRigidBody());
		rbc->setGravity(btVector3(0, 0, 0));
		rbc->setAngularVelocity(btVector3(0, 2, 0));
	}
}

void PickUpSystem::registerUI() {}
