#include "hydra/system/spawnersystem.hpp"

#include <hydra/ext/openmp.hpp>

#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/spawnercomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/movementcomponent.hpp>

using namespace Hydra::System;

Hydra::System::SpawnerSystem::SpawnerSystem()	{}

Hydra::System::SpawnerSystem::~SpawnerSystem()	{}

void Hydra::System::SpawnerSystem::tick(float delta)
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
				if (spawner->spawnGroup.size() <= 5)
				{
					if (spawner->spawnTimer >= 5)
					{
						auto alienSpawn = world::newEntity("AlienSpawn", world::root());
						auto a = alienSpawn->addComponent<Hydra::Component::EnemyComponent>();
						a->_enemyID = Hydra::Component::EnemyTypes::Alien;
						a->_damage = 4;
						a->_originalRange = 4;
						auto h = alienSpawn->addComponent<Hydra::Component::LifeComponent>();
						h->maxHP = 80;
						h->health = 80;
						auto m = alienSpawn->addComponent<Hydra::Component::MovementComponent>();
						m->movementSpeed = 8.0f;
						auto t = alienSpawn->addComponent<Hydra::Component::TransformComponent>();
						t->position = transform->position;
						t->scale = glm::vec3{ 2,2,2 };
						a->_scale = glm::vec3{ 2,2,2 };
						alienSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel1.mATTIC");
						spawner->spawnGroup.push_back(alienSpawn);
						spawner->spawnTimer = 0;
					}
				}
			}break;
			case Component::SpawnerType::RobotSpawner:
			{

			}break;
		}

	}
}

void Hydra::System::SpawnerSystem::registerUI()	{}
