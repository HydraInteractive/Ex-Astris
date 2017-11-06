#include <hydra/abilities/abilityHandler.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/soundfxcomponent.hpp>
#include <hydra/component/movementcomponent.hpp>

#include <hydra/ext/openmp.hpp>
#include <hydra/engine.hpp>
#include <btBulletDynamicsCommon.h>

using world = Hydra::World::World;

AbilityHandler::AbilityHandler(){
	_activeAbility = 0;
	_abilityList.push_back(&AbilityHandler::forcePushAbility);
	_abilityList.push_back(&AbilityHandler::grenadeAbility);
	_cooldownList.push_back(0);
	_cooldownList.push_back(0);
}

void AbilityHandler::addAbility(void(AbilityHandler::*newAbility)(Hydra::World::Entity*))
{
	_abilityList.push_back(newAbility);
	_cooldownList.push_back(0);
}

void AbilityHandler::useAbility(Hydra::World::Entity* playerEntity) {
	if (_cooldownList.size() > 0 && _cooldownList[_activeAbility] == 0)
	{
		(this->*_abilityList[_activeAbility])(playerEntity);
		
		//_cooldownList[_activeAbility] = 5;
		
		if (++_activeAbility >= _abilityList.size())
		{
			_activeAbility = 0;
		}
	}
}

void AbilityHandler::grenadeAbility(Hydra::World::Entity* playerEntity) {
	auto playerMovement = playerEntity->getComponent<Hydra::Component::MovementComponent>();
	auto playerTransform = playerEntity->getComponent<Hydra::Component::TransformComponent>();
	
	auto grenade = world::newEntity("Grenade", world::root());
	grenade->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SmallCargo.mATTIC");
	grenade->addComponent<Hydra::Component::SoundFxComponent>();
	grenade->addComponent<Hydra::Component::GrenadeComponent>();

	auto t = grenade->addComponent<Hydra::Component::TransformComponent>();
	t->position = playerTransform->position;
	t->scale = glm::vec3(0.5f);

	auto bulletPhysWorld = static_cast<Hydra::System::BulletPhysicsSystem*>(Hydra::IEngine::getInstance()->getState()->getPhysicsSystem());
	auto rbc = grenade->addComponent<Hydra::Component::RigidBodyComponent>();
	rbc->createBox(glm::vec3(0.5f,0.3f,0.3f), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER_PROJECTILE, 0.4f);
	auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
	bulletPhysWorld->enable(rbc.get());
	
	rigidBody->applyCentralForce(btVector3(playerMovement->direction.x, playerMovement->direction.y, playerMovement->direction.z) * 200);
	rigidBody->setFriction(0.0f);
}

void AbilityHandler::mineAbility(Hydra::World::Entity* playerEntity) {
	/*auto playerMovement = playerEntity->getComponent<Hydra::Component::MovementComponent>();
	auto playerTransform = playerEntity->getComponent<Hydra::Component::TransformComponent>();
	
	auto mine = world::newEntity("Mine", world::root());
	mine->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SmallCargo.mATTIC");

	auto m = mine->addComponent<Hydra::Component::MineComponent>();
	m->direction = playerMovement->direction;

	auto t = mine->addComponent<Hydra::Component::TransformComponent>();
	t->position = playerTransform->position;

	auto bulletPhysWorld = static_cast<Hydra::System::BulletPhysicsSystem*>(Hydra::IEngine::getInstance()->getState()->getPhysicsSystem());

	auto rbc = mine->addComponent<Hydra::Component::RigidBodyComponent>();
	rbc->createBox(glm::vec3(0.5f, 0.3f, 0.3f), 0.4f);

	auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
	bulletPhysWorld->enable(rbc.get());
	rigidBody->applyCentralForce(btVector3(m->direction.x, m->direction.y, m->direction.z) * 200);
	rigidBody->setFriction(0.75f);*/
}

void AbilityHandler::forcePushAbility(Hydra::World::Entity* playerEntity) {
	auto playerTransform = playerEntity->getComponent<Hydra::Component::TransformComponent>();
	
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
	world::getEntitiesWithComponents<Hydra::Component::TransformComponent, Hydra::Component::RigidBodyComponent>(entities);
	
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto t = entities[i]->getComponent<Hydra::Component::TransformComponent>();
		if (glm::distance(t->position, playerTransform->position) < 5.0f) {
			auto rbc = entities[i]->getComponent<Hydra::Component::RigidBodyComponent>();
			auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
			
			glm::vec3 forcePush = glm::normalize(t->position - playerTransform->position)*1000.0f;
			rigidBody->applyCentralForce(btVector3(forcePush.x, forcePush.y, forcePush.z));
		}
	}
}