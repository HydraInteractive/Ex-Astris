#include "hydra/system/spawnersystem.hpp"

#include <hydra/ext/openmp.hpp>

#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/spawnercomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>

using namespace Hydra::System;

SpawnerSystem::SpawnerSystem() {}

SpawnerSystem::~SpawnerSystem() {}

void SpawnerSystem::tick(float delta)
{
	using world = Hydra::World::World;

	//Process SystemComponent
	world::getEntitiesWithComponents<Component::SpawnerComponent, Component::LifeComponent, Component::TransformComponent>(entities);
#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto life = entities[i]->getComponent<Component::LifeComponent>();
		auto transform = entities[i]->getComponent<Component::TransformComponent>();
		auto spawner = entities[i]->getComponent<Component::SpawnerComponent>();

		spawner->spawnTimer += delta;

		switch (spawner->spawnerID)
		{
		case Component::SpawnerType::AlienSpawner:
		{
			if (spawner->spawnGroup.size() <= 4)
			{
				if (spawner->spawnTimer >= 5)
				{
					auto alienSpawn = world::newEntity("AlienSpawn", world::root());
					auto a = alienSpawn->addComponent <Hydra::Component::AIComponent> ();
					a->behaviour = std::make_shared<AlienBehaviour>(alienSpawn);
					a->damage = 4;
					a->radius = 2;
					a->behaviour->originalRange = 4;
					auto h = alienSpawn->addComponent<Hydra::Component::LifeComponent>();
					h->maxHP = 80;
					h->health = 80;
					auto m = alienSpawn->addComponent<Hydra::Component::MovementComponent>();
					m->movementSpeed = 8.0f;
					auto t = alienSpawn->addComponent<Hydra::Component::TransformComponent>();
					t->position = transform->position;
					t->scale = glm::vec3{ 2,2,2 };
					
					alienSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel1.mATTIC");
					spawner->spawnGroup.push_back(alienSpawn);
					spawner->spawnTimer = 0;
				}
			}
		}break;
		case Component::SpawnerType::RobotSpawner:
		{
			if (spawner->spawnGroup.size() <= 4)
			{
				if (spawner->spawnTimer >= 5)
				{
					auto robotSpawn = world::newEntity("RobotSpawn", world::root());
					auto a = robotSpawn->addComponent<Hydra::Component::AIComponent>();
					a->behaviour = std::make_shared<AlienBehaviour>(robotSpawn);
					a->damage = 8;
					a->radius = 1;
					a->behaviour->originalRange = 25;
					auto h = robotSpawn->addComponent<Hydra::Component::LifeComponent>();
					h->maxHP = 60;
					h->health = 60;
					auto m = robotSpawn->addComponent<Hydra::Component::MovementComponent>();
					m->movementSpeed = 4.0f;
					auto t = robotSpawn->addComponent<Hydra::Component::TransformComponent>();
					t->position = transform->position;
					t->scale = glm::vec3{ 1,1,1 };
					{
						auto weaponEntity = world::newEntity("Weapon", robotSpawn);
						weaponEntity->addComponent<Hydra::Component::WeaponComponent>();
					}

					robotSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/alphaGunModel.ATTIC");
					spawner->spawnGroup.push_back(robotSpawn);
					spawner->spawnTimer = 0;
				}
			}
		}break;
		}

	}
}

void SpawnerSystem::registerUI() {}