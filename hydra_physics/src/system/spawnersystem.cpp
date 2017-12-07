#include "hydra/system/spawnersystem.hpp"

#include <hydra/ext/openmp.hpp>

#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/spawnercomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/engine.hpp>

#include <btBulletDynamicsCommon.h>

using namespace Hydra::System;

SpawnerSystem::SpawnerSystem() {}

SpawnerSystem::~SpawnerSystem() {}

void SpawnerSystem::tick(float delta)
{
	using world = Hydra::World::World;

	//Process SystemComponent
	world::getEntitiesWithComponents<Component::SpawnerComponent, Component::TransformComponent, Component::LifeComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto transform = entities[i]->getComponent<Component::TransformComponent>();
		auto spawner = entities[i]->getComponent<Component::SpawnerComponent>();
		auto life = entities[i]->getComponent<Component::LifeComponent>();

		spawner->spawnTimer += delta;
		auto playerTrans = spawner->getPlayerEntity()->getComponent<Component::TransformComponent>();

		for (size_t i = 0; i < spawner->spawnGroup.size(); i++)
		{
			if (Hydra::World::World::getEntity(spawner->spawnGroup[i]) == NULL)
			{
				spawner->spawnGroup.erase(spawner->spawnGroup.begin() + i);
				spawner->spawnCounter++;
			}
		}

		if (spawner->spawnCounter >= 9)
		{
			life->health = 0;
		}

		if (glm::length(transform->position - playerTrans->position) < 50.0f)
		{
			switch (spawner->spawnerID)
			{
			case Component::SpawnerType::AlienSpawner:
			{
				if (spawner->spawnGroup.size() <= 2)
				{
					if (spawner->spawnTimer >= 10)
					{
						auto alienSpawn = world::newEntity("SlowAlien2", world::root());
						alienSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
						auto as = alienSpawn->addComponent<Hydra::Component::AIComponent>();
						as->behaviour = std::make_shared<AlienBehaviour>(alienSpawn);
						as->behaviour->setPathMap(spawner->map);
						as->damage = 4;
						as->behaviour->originalRange = 4.0f;
						as->behaviour->savedRange = as->behaviour->originalRange;
						as->radius = 1;


						auto hs = alienSpawn->addComponent<Hydra::Component::LifeComponent>();
						hs->maxHP = 80;
						hs->health = 80;

						auto ws = alienSpawn->addComponent<Hydra::Component::WeaponComponent>();
						ws->bulletSpread = 0.2f;
						ws->bulletsPerShot = 1;
						ws->damage = 4;
						ws->bulletSize = 0.3;
						ws->maxmagammo = 0;
						ws->currmagammo = 0;
						ws->maxammo = 0;

						auto ms = alienSpawn->addComponent<Hydra::Component::MovementComponent>();
						ms->movementSpeed = 5.0f;

						auto ts = alienSpawn->addComponent<Hydra::Component::TransformComponent>();
						ts->position.x = transform->position.x;
						ts->position.y = 1.0;
						ts->position.z = transform->position.z;
						ts->scale = glm::vec3{ 1,1,1 };

						auto rgbcs = alienSpawn->addComponent<Hydra::Component::RigidBodyComponent>();
						rgbcs->createBox(glm::vec3(0.4f, 0.8f, 0.4f) * ts->scale, glm::vec3(0, 1.8, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
							0, 0, 0.6f, 1.0f);
						rgbcs->createCapsuleY(0.3f, 0.8f * ts->scale.y, glm::vec3(0, 3.0, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_HEAD, 10000,
							0, 0, 0.0f, 0);
						rgbcs->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
						rgbcs->setAngularForce(glm::vec3(0));
						spawner->spawnGroup.push_back(alienSpawn->id);
						spawner->spawnTimer = 0;

						static_cast<Hydra::System::BulletPhysicsSystem*>(Hydra::IEngine::getInstance()->getState()->getPhysicsSystem())->enable(rgbcs.get());
					}
				}
			}break;
			case Component::SpawnerType::RobotSpawner:
			{
				if (spawner->spawnGroup.size() <= 2)
				{
					if (spawner->spawnTimer >= 10)
					{
						auto robotSpawn = world::newEntity("Robot2", world::root());
						robotSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/RobotModel.mATTIC");
						auto as = robotSpawn->addComponent<Hydra::Component::AIComponent>();
						as->behaviour = std::make_shared<RobotBehaviour>(robotSpawn);
						as->behaviour->setPathMap(spawner->map);
						as->damage = 7;
						as->behaviour->originalRange = 18;
						as->behaviour->savedRange = as->behaviour->originalRange;
						as->radius = 1;

						auto hs = robotSpawn->addComponent<Hydra::Component::LifeComponent>();
						hs->maxHP = 70;
						hs->health = 70;

						auto ws = robotSpawn->addComponent<Hydra::Component::WeaponComponent>();
						ws->bulletSpread = 0.3f;
						ws->fireRateRPM = 50;
						ws->bulletsPerShot = 1;
						ws->damage = 7;
						ws->bulletSize = 0.3;
						ws->maxmagammo = 0;
						ws->currmagammo = 0;
						ws->maxammo = 0;

						auto ms = robotSpawn->addComponent<Hydra::Component::MovementComponent>();
						ms->movementSpeed = 3.0f;
						auto ts = robotSpawn->addComponent<Hydra::Component::TransformComponent>();
						ts->position.x = transform->position.x;
						ts->position.y = 1.0;
						ts->position.z = transform->position.z;
						ts->scale = glm::vec3{ 1,1,1 };

						auto rgbcs = robotSpawn->addComponent<Hydra::Component::RigidBodyComponent>();
						rgbcs->createBox(glm::vec3(0.4f, 1.0f, 0.4f) * ts->scale, glm::vec3(0, 1.6, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
							0, 0, 0.6f, 1.0f);
						rgbcs->createCapsuleY(0.3f, 0.8f * ts->scale.y, glm::vec3(0, 3.1, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_HEAD, 10000,
							0, 0, 0.0f, 0);
						rgbcs->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
						rgbcs->setAngularForce(glm::vec3(0));
						spawner->spawnGroup.push_back(robotSpawn->id);
						spawner->spawnTimer = 0;

						static_cast<Hydra::System::BulletPhysicsSystem*>(Hydra::IEngine::getInstance()->getState()->getPhysicsSystem())->enable(rgbcs.get());
					}
				}
			}break;
			}
		}

	}

	entities.clear();
}

void SpawnerSystem::registerUI() {}

