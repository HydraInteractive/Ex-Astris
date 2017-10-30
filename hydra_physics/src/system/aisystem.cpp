#include <hydra/system/aisystem.hpp>

#include <hydra/ext/openmp.hpp>

#include <hydra/component/aicomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/movementcomponent.hpp>

using namespace Hydra::System;

AISystem::AISystem() {}
AISystem::~AISystem() {}

void AISystem::tick(float delta) {
	using world = Hydra::World::World;

	//Process AiComponent
	world::getEntitiesWithComponents<Component::AIComponent, Component::TransformComponent, Component::LifeComponent>(entities);
#pragma omp parallel for
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto enemy = entities[i]->getComponent<Component::AIComponent>();
		enemy->behaviour->run(delta);
		//switch (enemy->_enemyID)
		//{
		//case Component::EnemyTypes::Alien:
		//{
		//	switch (enemy->_pathState)
		//	{
		//	case Component::PathState::IDLE:
		//	{
		//		drawObject->drawObject->mesh->setAnimationIndex(0);
		//		//Play the idle animation
		//		if (playerTransform->position.x > enemy->_mapOffset.x && playerTransform->position.x < WORLD_SIZE && playerTransform->position.z > enemy->_mapOffset.z && playerTransform->position.z < WORLD_SIZE)
		//		{
		//			if (glm::length(transform->position - playerTransform->position) < 50)
		//			{
		//				enemy->_pathState = Component::PathState::SEARCHING;
		//				enemy->_timer = 0;
		//			}
		//		}
		//	}break;
		//	case Component::PathState::SEARCHING:
		//	{
		//		//While the enemy is searching, play the walking animation
		//		drawObject->drawObject->mesh->setAnimationIndex(1);
		//		if (enemy->_timer >= 5)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//		if (glm::length(transform->position - playerTransform->position) < enemy->_range)
		//		{
		//			enemy->_isAtGoal = true;
		//			enemy->_pathFinding->foundGoal = true;
		//			enemy->_pathState = Component::PathState::ATTACKING;
		//		}
		//		if (playerTransform->position.x <= enemy->_mapOffset.x || playerTransform->position.x >= WORLD_SIZE || playerTransform->position.z <= enemy->_mapOffset.z || playerTransform->position.z >= WORLD_SIZE)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//		enemy->_pathFinding->intializedStartGoal = false;
		//		enemy->_pathFinding->findPath(transform->position, playerTransform->position, enemy->_map);
		//		enemy->_isAtGoal = false;
		//		if (enemy->_pathFinding->foundGoal)
		//		{
		//			if (!enemy->_pathFinding->_pathToEnd.empty())
		//			{
		//				enemy->_targetPos = enemy->_pathFinding->_pathToEnd[0];
		//			}
		//			enemy->_pathState = Component::PathState::FOUND_GOAL;
		//			enemy->_newPathTimer = 0;
		//		}
		//	}break;
		//	case Component::PathState::FOUND_GOAL:
		//	{
		//		if (!enemy->_isAtGoal)
		//		{
		//			if (!enemy->_pathFinding->_pathToEnd.empty())
		//			{
		//				glm::vec3 targetDistance = enemy->_pathFinding->nextPathPos(transform->position, enemy->getRadius()) - transform->position;
		//				enemy->_angle = atan2(targetDistance.x, targetDistance.z);
		//				enemy->_rotation = glm::angleAxis(enemy->_angle, glm::vec3(0, 1, 0));
		//				glm::vec3 direction = glm::normalize(targetDistance);
		//				movement->velocity.x = (movement->movementSpeed * direction.x) * delta;
		//				movement->velocity.z = (movement->movementSpeed * direction.z) * delta;
		//				if (glm::length(transform->position - playerTransform->position) <= enemy->_range)
		//				{
		//					enemy->_isAtGoal = true;
		//					enemy->_pathFinding->foundGoal = true;
		//					enemy->_pathState = Component::PathState::ATTACKING;
		//				}
		//				if (glm::length(transform->position - enemy->_targetPos) <= 1.0f)
		//				{
		//					enemy->_pathState = Component::PathState::SEARCHING;
		//					enemy->_timer = 0;
		//				}
		//				else if (glm::length(playerTransform->position.x - enemy->_targetPos.x) > 25.0f || glm::length(playerTransform->position.z - enemy->_targetPos.z) > 25.0f)
		//				{
		//					enemy->_pathState = Component::PathState::SEARCHING;
		//					enemy->_timer = 0;
		//				}
		//			}
		//		}
		//		if (enemy->_newPathTimer >= 4)
		//		{
		//			enemy->_pathState = Component::PathState::SEARCHING;
		//			enemy->_timer = 0;
		//		}
		//		if (playerTransform->position.x <= enemy->_mapOffset.x || playerTransform->position.x >= WORLD_SIZE || playerTransform->position.z <= enemy->_mapOffset.z || playerTransform->position.z >= WORLD_SIZE)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//	}break;
		//	case Component::PathState::ATTACKING:
		//	{
		//		//When the enemy attack, start the attack animation
		//		drawObject->drawObject->mesh->setAnimationIndex(2);
		//		if (glm::length(transform->position - playerTransform->position) >= enemy->_range)
		//		{
		//			enemy->_pathState = Component::PathState::SEARCHING;
		//			enemy->_timer = 0;
		//		}
		//		else
		//		{
		//			std::mt19937 rng(enemy->rd());
		//			std::uniform_int_distribution<> randDmg(enemy->_damage - 1, enemy->_damage + 2);
		//			if (enemy->_attackTimer > 1.5)
		//			{
		//				//player->applyDamage(randDmg(rng));
		//				enemy->_attackTimer = 0;
		//			}
		//			glm::vec3 playerDir = playerTransform->position - transform->position;
		//			playerDir = glm::normalize(playerDir);
		//			enemy->_angle = atan2(playerDir.x, playerDir.z);
		//			enemy->_rotation = glm::angleAxis(enemy->_angle, glm::vec3(0, 1, 0));
		//		}
		//	}break;
		//	}
		//	enemy->_playerSeen = enemy->_checkLOS(enemy->_map, transform->position, playerTransform->position);
		//	if (enemy->_playerSeen == false)
		//	{
		//		if (enemy->_range > 0.5f)
		//		{
		//			enemy->_range -= 0.5f;
		//		}
		//	}
		//	else
		//	{
		//		enemy->_range = enemy->_originalRange;
		//	}
		//	transform->position += glm::vec3(movement->velocity.x, movement->velocity.y, movement->velocity.z);
		//	transform->setPosition(transform->position);
		//	transform->setRotation(enemy->_rotation);
		//}break;
		//case Component::EnemyTypes::Robot:
		//{
		//	switch (enemy->_pathState)
		//	{
		//	case Component::PathState::IDLE:
		//	{
		//		if (playerTransform->position.x > enemy->_mapOffset.x && playerTransform->position.x < WORLD_SIZE && playerTransform->position.z > enemy->_mapOffset.z && playerTransform->position.z < WORLD_SIZE)
		//		{
		//			if (glm::length(transform->position - playerTransform->position) < 50)
		//			{
		//				enemy->_timer = 0;
		//				enemy->_pathState = Component::PathState::SEARCHING;
		//			}
		//		}
		//	}break;
		//	case Component::PathState::SEARCHING:
		//	{
		//		if (enemy->_timer >= 5)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//		if (glm::length(transform->position - playerTransform->position) < enemy->_range)
		//		{
		//			enemy->_isAtGoal = true;
		//			enemy->_pathState = Component::PathState::ATTACKING;
		//		}
		//		if (playerTransform->position.x <= enemy->_mapOffset.x || playerTransform->position.x >= WORLD_SIZE || playerTransform->position.z <= enemy->_mapOffset.z || playerTransform->position.z >= WORLD_SIZE)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//		enemy->_pathFinding->intializedStartGoal = false;
		//		enemy->_pathFinding->findPath(transform->position, playerTransform->position, enemy->_map);
		//		enemy->_isAtGoal = false;
		//		if (enemy->_pathFinding->foundGoal)
		//		{
		//			if (!enemy->_pathFinding->_pathToEnd.empty())
		//			{
		//				enemy->_targetPos = enemy->_pathFinding->_pathToEnd[0];
		//			}
		//			enemy->_pathState = Component::PathState::FOUND_GOAL;
		//			enemy->_newPathTimer = 0;
		//		}
		//	}break;
		//	case Component::PathState::FOUND_GOAL:
		//	{
		//		if (!enemy->_isAtGoal)
		//		{
		//			if (!enemy->_pathFinding->_pathToEnd.empty())
		//			{
		//				glm::vec3 targetDistance = enemy->_pathFinding->nextPathPos(transform->position, enemy->getRadius()) - transform->position;
		//				enemy->_angle = atan2(targetDistance.x, targetDistance.z);
		//				enemy->_rotation = glm::angleAxis(enemy->_angle, glm::vec3(0, 1, 0));
		//				glm::vec3 direction = glm::normalize(targetDistance);
		//				movement->velocity.x = (movement->movementSpeed * direction.x) * delta;
		//				movement->velocity.z = (movement->movementSpeed * direction.z) * delta;
		//				if (glm::length(transform->position - playerTransform->position) < enemy->_range)
		//				{
		//					enemy->_isAtGoal = true;
		//					enemy->_pathFinding->foundGoal = true;
		//					enemy->_pathState = Component::PathState::ATTACKING;
		//				}
		//				if (glm::length(transform->position - enemy->_targetPos) <= 8.0f)
		//				{
		//					enemy->_pathState = Component::PathState::SEARCHING;
		//					enemy->_timer = SDL_GetTicks();
		//				}
		//				else if (glm::length(playerTransform->position - enemy->_targetPos) > 25.0f)
		//				{
		//					enemy->_pathState = Component::PathState::SEARCHING;
		//					enemy->_timer = SDL_GetTicks();
		//				}
		//			}
		//		}
		//		if (enemy->_newPathTimer >= 4)
		//		{
		//			enemy->_pathState = Component::PathState::SEARCHING;
		//			enemy->_timer = SDL_GetTicks();
		//		}
		//		if (playerTransform->position.x <= enemy->_mapOffset.x || playerTransform->position.x >= WORLD_SIZE || playerTransform->position.z <= enemy->_mapOffset.z || playerTransform->position.z >= WORLD_SIZE)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//	}break;
		//	case Component::PathState::ATTACKING:
		//	{
		//		if (glm::length(transform->position - playerTransform->position) > enemy->_range)
		//		{
		//			enemy->_pathState = Component::PathState::SEARCHING;
		//			enemy->_timer = 0;
		//		}
		//		else
		//		{
		//			glm::vec3 playerDir = playerTransform->position - transform->position;
		//			playerDir = glm::normalize(playerDir);
		//			weapon->shoot(transform->position, -playerDir, glm::quat(), 8.0f);
		//			enemy->_angle = atan2(playerDir.x, playerDir.z);
		//			enemy->_rotation = glm::angleAxis(enemy->_angle, glm::vec3(0, 1, 0));
		//		}
		//	}break;
		//	}
		//	enemy->_playerSeen = enemy->_checkLOS(enemy->_map, transform->position, playerTransform->position);
		//	if (enemy->_playerSeen == false)
		//	{
		//		if (enemy->_range > 2.0f)
		//		{
		//			enemy->_range -= 1.0f;
		//		}
		//	}
		//	else
		//	{
		//		enemy->_range = enemy->_originalRange;
		//	}
		//	transform->position += glm::vec3(movement->velocity.x, movement->velocity.y, movement->velocity.z);
		//	transform->setPosition(transform->position);
		//	transform->setRotation(enemy->_rotation);
		//}break;
		//case Component::EnemyTypes::AlienSpawner:
		//{
		//	if (enemy->_spawnGroup.size() <= 5)
		//	{
		//		if (enemy->_spawnTimer >= 10)
		//		{
		//		}
		//	}
		//}break;
		//case Component::EnemyTypes::RobotSpawner:
		//{
		//	if (enemy->_spawnGroup.size() <= 5)
		//	{
		//		if (enemy->_spawnTimer >= 10)
		//		{
		//		}
		//	}
		//}break;
		//case Component::EnemyTypes::AlienBossBehaviour:
		//{
		//	switch (enemy->_pathState)
		//	{
		//	case Component::PathState::IDLE:
		//	{
		//		if (playerTransform->position.x > enemy->_mapOffset.x && playerTransform->position.x < WORLD_SIZE && playerTransform->position.z > enemy->_mapOffset.z && playerTransform->position.z < WORLD_SIZE)
		//		{
		//			if (glm::length(transform->position - playerTransform->position) < 50)
		//			{
		//				enemy->_timer = 0;
		//				enemy->_pathState = Component::PathState::SEARCHING;
		//			}
		//		}
		//	}break;
		//	case Component::PathState::SEARCHING:
		//	{
		//		if (enemy->_timer >= 5)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//		if (glm::length(transform->position - playerTransform->position) < enemy->_range)
		//		{
		//			enemy->_isAtGoal = true;
		//			enemy->_pathState = Component::PathState::ATTACKING;
		//		}
		//		if (playerTransform->position.x <= enemy->_mapOffset.x || playerTransform->position.x >= WORLD_SIZE || playerTransform->position.z <= enemy->_mapOffset.z || playerTransform->position.z >= WORLD_SIZE)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//		enemy->_pathFinding->intializedStartGoal = false;
		//		enemy->_pathFinding->findPath(transform->position, playerTransform->position, enemy->_map);
		//		enemy->_isAtGoal = false;
		//		if (enemy->_pathFinding->foundGoal)
		//		{
		//			if (!enemy->_pathFinding->_pathToEnd.empty())
		//			{
		//				enemy->_targetPos = enemy->_pathFinding->_pathToEnd[0];
		//			}
		//			enemy->_pathState = Component::PathState::FOUND_GOAL;
		//			enemy->_newPathTimer = 0;
		//		}
		//	}break;
		//	case Component::PathState::FOUND_GOAL:
		//	{
		//		if (!enemy->_isAtGoal)
		//		{
		//			if (!enemy->_pathFinding->_pathToEnd.empty())
		//			{
		//				glm::vec3 targetDistance = enemy->_pathFinding->nextPathPos(transform->position, enemy->getRadius()) - transform->position;
		//				enemy->_angle = atan2(targetDistance.x, targetDistance.z);
		//				enemy->_rotation = glm::angleAxis(enemy->_angle, glm::vec3(0, 1, 0));
		//				glm::vec3 direction = glm::normalize(targetDistance);
		//				movement->velocity.x = (movement->movementSpeed * direction.x) * delta;
		//				movement->velocity.z = (movement->movementSpeed * direction.z) * delta;
		//				if (glm::length(transform->position - playerTransform->position) < enemy->_range)
		//				{
		//					enemy->_isAtGoal = true;
		//					enemy->_pathFinding->foundGoal = true;
		//					enemy->_pathState = Component::PathState::ATTACKING;
		//				}
		//				if (glm::length(transform->position - enemy->_targetPos) <= 6.0f)
		//				{
		//					enemy->_pathState = Component::PathState::SEARCHING;
		//					enemy->_timer = 0;
		//				}
		//				else if (glm::length(playerTransform->position - enemy->_targetPos) > 25.0f)
		//				{
		//					enemy->_pathState = Component::PathState::SEARCHING;
		//					enemy->_timer = 0;
		//				}
		//			}
		//		}
		//		if (enemy->_newPathTimer >= 4)
		//		{
		//			enemy->_pathState = Component::PathState::SEARCHING;
		//			enemy->_timer = 0;
		//		}
		//		if (playerTransform->position.x <= enemy->_mapOffset.x || playerTransform->position.x >= WORLD_SIZE || playerTransform->position.z <= enemy->_mapOffset.z || playerTransform->position.z >= WORLD_SIZE)
		//		{
		//			enemy->_pathState = Component::PathState::IDLE;
		//		}
		//	}break;
		//	case Component::PathState::ATTACKING:
		//	{
		//		if (glm::length(transform->position - playerTransform->position) > enemy->_range && enemy->_stunned == false)
		//		{
		//			enemy->_pathState = Component::PathState::SEARCHING;
		//			enemy->_timer = 0;
		//		}
		//		else
		//		{
		//			glm::vec3 playerDir = playerTransform->position - transform->position;
		//			playerDir = glm::normalize(playerDir);
		//			switch (enemy->_bossPhase)
		//			{
		//			case Component::BossPhase::CLAWING:
		//			{
		//				enemy->_range = 9.0f;
		//				std::mt19937 rng(enemy->rd());
		//				std::uniform_int_distribution<> randDmg(enemy->_damage - 1, enemy->_damage + 2);
		//				if (enemy->_attackTimer >= 3)
		//				{
		//					//player->applyDamage(randDmg(rng));
		//					enemy->_attackTimer = 0;
		//				}
		//			}break;
		//			case Component::BossPhase::SPITTING:
		//			{
		//				enemy->_range = 30.0f;
		//				weapon->shoot(transform->position, -playerDir, glm::quat(), 15.0f);
		//			}break;
		//			case Component::BossPhase::SPAWNING:
		//			{
		//				enemy->_range = 30.0f;
		//				/*if (_spawnAmount <= 3)
		//				{
		//				if (_spawnTimer > 2)
		//				{
		//				_spawnAmount++;
		//				_spawnTimer = 0;
		//				}
		//				}*/
		//			}break;
		//			case Component::BossPhase::CHILLING:
		//			{
		//				enemy->_range = 30.0f;
		//				if (enemy->_stunTimer > 10)
		//				{
		//					if (!enemy->_stunned) { enemy->_stunned = true; }
		//					else
		//					{
		//						enemy->_stunned = false;
		//						enemy->_bossPhase = Component::BossPhase::CLAWING;
		//					}
		//					enemy->_stunTimer = 0;
		//				}
		//			}break;
		//			}
		//			if (enemy->_stunned == false)
		//			{
		//				enemy->_angle = atan2(playerDir.x, playerDir.z);
		//				enemy->_rotation = glm::angleAxis(enemy->_angle, glm::vec3(0, 1, 0));
		//			}
		//		}
		//	}break;
		//	}
		//	transform->position += glm::vec3(movement->velocity.x, movement->velocity.y, movement->velocity.z);
		//	transform->setPosition(transform->position);
		//	transform->setRotation(enemy->_rotation);
		//}break;
		//default:
		//	break;
		//}

		//debug for pathfinding
		//int tempX = transform->getPosition().x;
		//int tempZ = transform->getPosition().z;
		//enemy->_map[tempX][tempZ] = 2;
		//for (int i = 0; i < enemy->_pathFinding->_visitedList.size(); i++)
		//{
		//	enemy->_map[(int)enemy->_pathFinding->_visitedList[i]->m_xcoord][(int)enemy->_pathFinding->_visitedList[i]->m_zcoord] = 3;
		//}
		//for (int i = 0; i < enemy->_pathFinding->_openList.size(); i++)
		//{
		//	enemy->_map[(int)enemy->_pathFinding->_openList[i]->m_xcoord][(int)enemy->_pathFinding->_openList[i]->m_zcoord] = 3;
		//}
		//for (int i = 0; i < enemy->_pathFinding->_pathToEnd.size(); i++)
		//{
		//	enemy->_map[(int)enemy->_pathFinding->_pathToEnd[i].x][(int)enemy->_pathFinding->_pathToEnd[i].z] = 3;
		//}
	}
}

void AISystem::registerUI() {}
