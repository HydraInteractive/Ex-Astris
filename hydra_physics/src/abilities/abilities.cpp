#include <hydra/abilities/abilities.hpp>

void GrenadeAbility::useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity) {
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
	rbc->createBox(glm::vec3(0.5f, 0.3f, 0.3f), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER_PROJECTILE, 0.4f);
	auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
	bulletPhysWorld->enable(rbc.get());

	rigidBody->applyCentralForce(btVector3(playerMovement->direction.x, playerMovement->direction.y, playerMovement->direction.z) * 200);
	rigidBody->setFriction(0.0f);
}

void MineAbility::useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity){
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

void forcePushAbility::useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity) {
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

void BulletSprayAbillity::useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity) {
	activeTimer = 1.5f;
}

void BulletSprayAbillity::tick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity)
{
	auto playerTransform = playerEntity->getComponent<Hydra::Component::TransformComponent>();
	auto playerMovement = playerEntity->getComponent<Hydra::Component::MovementComponent>();

	for (size_t i = 0; i < 6; i++){
		auto bullet = world::newEntity("Bullet", world::rootID);
		bullet->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/SmallCargo.mATTIC");

		float phi = ((float)rand() / (float)(RAND_MAX)) * (2.0f*3.14f);
		float distance = ((float)rand() / (float)(RAND_MAX)) * 0.4f;
		float theta = ((float)rand() / (float)(RAND_MAX)) * 3.14f;

		glm::vec3 bulletDirection = playerMovement->direction;
		bulletDirection.x += distance * sin(theta) * cos(phi);
		bulletDirection.y += distance * sin(theta) * sin(phi);
		bulletDirection.z += distance * cos(theta);
		bulletDirection = glm::normalize(bulletDirection);

		auto b = bullet->addComponent<Hydra::Component::BulletComponent>();
		b->direction = bulletDirection;

		auto t = bullet->addComponent<Hydra::Component::TransformComponent>();
		t->position = playerTransform->position;
		t->scale = glm::vec3(0.5f);

		auto bulletPhysWorld = static_cast<Hydra::System::BulletPhysicsSystem*>(Hydra::IEngine::getInstance()->getState()->getPhysicsSystem());

		auto rbc = bullet->addComponent<Hydra::Component::RigidBodyComponent>();
		rbc->createBox(glm::vec3(0.5f), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_PLAYER_PROJECTILE, 0.0095f);
		auto rigidBody = static_cast<btRigidBody*>(rbc->getRigidBody());
		bulletPhysWorld->enable(rbc.get());
		rigidBody->applyCentralForce(btVector3(b->direction.x, b->direction.y, b->direction.z) * 300);
		rigidBody->setActivationState(DISABLE_DEACTIVATION);
		rigidBody->setGravity(btVector3(0, 0, 0));
	}
}