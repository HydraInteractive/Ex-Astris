#include "hydra/pathing/behaviour.hpp"

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>

Behaviour::Behaviour(std::shared_ptr<Hydra::World::Entity> enemy)
{
	thisEnemy.entity = enemy;
	refreshComponents();

	pathFinding = std::make_shared<PathFinding>();

}
Behaviour::Behaviour()
{

}

Behaviour::~Behaviour()
{

}

void Behaviour::setEnemyEntity(std::shared_ptr<Hydra::World::Entity> enemy)
{
	thisEnemy.entity = enemy;
	refreshComponents();
}

void Behaviour::setTargetPlayer(std::shared_ptr<Hydra::World::Entity> player)
{
	targetPlayer.entity = player;
	targetPlayer.life = player->getComponent<Hydra::Component::LifeComponent>();
	targetPlayer.transform = player->getComponent<Hydra::Component::TransformComponent>();
}
//Sets all components without setting new entities, use after adding new components to either entity
void Behaviour::refreshComponents()
{
	thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>();
	thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>();
	thisEnemy.drawObject = thisEnemy.entity->getComponent<Hydra::Component::DrawObjectComponent>();
	thisEnemy.weapon = thisEnemy.entity->getComponent<Hydra::Component::WeaponComponent>();
	thisEnemy.life = thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>();
	thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>();
	
	targetPlayer.entity = thisEnemy.ai->getPlayerEntity();
	targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>();
	targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>();
}

unsigned int Behaviour::idleState(float dt)
{
	thisEnemy.drawObject->drawObject->mesh->setAnimationIndex(0);
	//Play the idle animation
	if (targetPlayer.transform->position.x > thisEnemy.ai->mapOffset.x && targetPlayer.transform->position.x < WORLD_SIZE && targetPlayer.transform->position.z > thisEnemy.ai->mapOffset.z && targetPlayer.transform->position.z < WORLD_SIZE)
	{
		if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) < 50)
		{
			thisEnemy.ai->_timer = 0;
			return SEARCHING;
			
		}
	}
	return state;
}

unsigned int Behaviour::searchingState(float dt)
{
	//While the enemy is searching, play the walking animation
	thisEnemy.drawObject->drawObject->mesh->setAnimationIndex(1);
	if (thisEnemy.ai->_timer >= 5)
	{
		state = IDLE;
	}
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) < thisEnemy.ai->_range)
	{
		thisEnemy.ai->_isAtGoal = true;
		pathFinding->foundGoal = true;
		state = ATTACKING;
	}

	if (targetPlayer.transform->position.x <= thisEnemy.ai->mapOffset.x || targetPlayer.transform->position.x >= WORLD_SIZE || targetPlayer.transform->position.z <= thisEnemy.ai->mapOffset.z || targetPlayer.transform->position.z >= WORLD_SIZE)
	{
		state = IDLE;
	}
	pathFinding->intializedStartGoal = false;
	pathFinding->findPath(thisEnemy.transform->position, targetPlayer.transform->position, thisEnemy.ai->_map);
	thisEnemy.ai->_isAtGoal = false;


	if (pathFinding->foundGoal)
	{
		if (!pathFinding->_pathToEnd.empty())
		{
			thisEnemy.ai->_targetPos = pathFinding->_pathToEnd[0];
		}
		thisEnemy.ai->_newPathTimer = 0;
		return FOUND_GOAL;
	}
	return state;
}

unsigned int Behaviour::foundState(float dt)
{
	if (!thisEnemy.ai->_isAtGoal)
	{
		if (!pathFinding->_pathToEnd.empty())
		{
			glm::vec3 targetDistance = pathFinding->nextPathPos(thisEnemy.transform->position, thisEnemy.ai->getRadius()) - thisEnemy.transform->position;

			thisEnemy.ai->_angle = atan2(targetDistance.x, targetDistance.z);
			thisEnemy.ai->_rotation = glm::angleAxis(thisEnemy.ai->_angle, glm::vec3(0, 1, 0));

			glm::vec3 direction = glm::normalize(targetDistance);

			thisEnemy.movement->velocity.x = (thisEnemy.movement->movementSpeed * direction.x) * dt;
			thisEnemy.movement->velocity.z = (thisEnemy.movement->movementSpeed * direction.z) * dt;

			if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) <= thisEnemy.ai->_range)
			{
				thisEnemy.ai->_isAtGoal = true;
				pathFinding->foundGoal = true;
				state = ATTACKING;
			}

			if (glm::length(thisEnemy.transform->position - thisEnemy.ai->_targetPos) <= 3.0f)
			{
				state = SEARCHING;
				thisEnemy.ai->_timer = 0;
			}
			else if (glm::length(targetPlayer.transform->position.x - thisEnemy.ai->_targetPos.x) > 25.0f || glm::length(targetPlayer.transform->position.z - thisEnemy.ai->_targetPos.z) > 25.0f)
			{
				state = SEARCHING;
				thisEnemy.ai->_timer = 0;
			}
		}
	}

	if (thisEnemy.ai->_newPathTimer >= 4)
	{
		state = SEARCHING;
		thisEnemy.ai->_timer = 0;
	}

	if (targetPlayer.transform->position.x <= thisEnemy.ai->mapOffset.x || targetPlayer.transform->position.x >= WORLD_SIZE || targetPlayer.transform->position.z <= thisEnemy.ai->mapOffset.z || targetPlayer.transform->position.z >= WORLD_SIZE)
	{
		return IDLE;
	}
	return state;
}

unsigned int Behaviour::attackingState(float dt)
{
	//When the enemy attack, start the attack animation
	thisEnemy.drawObject->drawObject->mesh->setAnimationIndex(2);
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) >= thisEnemy.ai->_range)
	{
		thisEnemy.ai->_timer = 0;
		return SEARCHING;
	}
	else
	{
		std::mt19937 rng(thisEnemy.ai->rd());
		std::uniform_int_distribution<> randDmg(thisEnemy.ai->_damage - 1, thisEnemy.ai->_damage + 2);
		if (thisEnemy.ai->_attackTimer > 1.5)
		{
			//player->applyDamage(randDmg(rng));
			thisEnemy.ai->_attackTimer = 0;
		}

		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		thisEnemy.ai->_angle = atan2(playerDir.x, playerDir.z);
		thisEnemy.ai->_rotation = glm::angleAxis(thisEnemy.ai->_angle, glm::vec3(0, 1, 0));
	}
	return state;
}

void Behaviour::executeTransforms()
{
	/*thisEnemy.ai->_playerSeen = thisEnemy.ai->_checkLOS(thisEnemy.ai->_map, thisEnemy.transform->position, targetPlayer.transform->position);*/

	/*if (thisEnemy.ai->_playerSeen == false)
	{
		if (thisEnemy.ai->_range > 0.5f)
		{
			thisEnemy.ai->_range -= 0.5f;
		}
	}
	else
	{*/
		thisEnemy.ai->_range = thisEnemy.ai->_originalRange;
	//}

	thisEnemy.transform->position += glm::vec3(thisEnemy.movement->velocity.x, thisEnemy.movement->velocity.y, thisEnemy.movement->velocity.z);
	thisEnemy.transform->setPosition(thisEnemy.transform->position);
	thisEnemy.transform->setRotation(thisEnemy.ai->_rotation);

	/*if (thisEnemy.transform->position.x != _oldMapPosX && thisEnemy.transform->position.z != _oldMapPosZ)
	{
		_map[_oldMapPosX][_oldMapPosZ] = 0;
		if (thisEnemy.transform->position.x <= 0 || thisEnemy.transform->position.z <= 0)
		{
			_oldMapPosX = thisEnemy.transform->position.x - mapOffset.x;
			_oldMapPosZ = thisEnemy.transform->position.z - mapOffset.z;
		}
		else
		{
			_oldMapPosX = thisEnemy.transform->position.x;
			_oldMapPosZ = thisEnemy.transform->position.z;
		}
		_map[_oldMapPosX][_oldMapPosZ] = 2;
	}
	else if (thisEnemy.transform->position.x != _oldMapPosX && thisEnemy.transform->position.z == _oldMapPosZ)
	{
		_map[_oldMapPosX][_oldMapPosZ] = 0;
		if (thisEnemy.transform->position.x <= 0 || thisEnemy.transform->position.z <= 0)
		{
			_oldMapPosX = thisEnemy.transform->position.x + mapOffset.x;
		}
		else
		{
			_oldMapPosX = thisEnemy.transform->position.x;
		}
		_map[_oldMapPosX][_oldMapPosZ] = 2;
	}
	else if (thisEnemy.transform->position.z != _oldMapPosZ && thisEnemy.transform->position.x == _oldMapPosX)
	{
		_map[_oldMapPosX][_oldMapPosZ] = 0;
		if (thisEnemy.transform->position.x <= 0 || thisEnemy.transform->position.z <= 0)
		{
			_oldMapPosZ = thisEnemy.transform->position.z + mapOffset.z;
		}
		else
		{
			_oldMapPosZ = thisEnemy.transform->position.z;
		}
		_map[_oldMapPosX][_oldMapPosZ] = 2;
	}*/
}

AlienBehaviour::AlienBehaviour(std::shared_ptr<Hydra::World::Entity> enemy) : Behaviour(enemy)
{
	this->type = Type::ALIEN;
}

AlienBehaviour::AlienBehaviour()
{
	this->type = Type::ALIEN;
}

AlienBehaviour::~AlienBehaviour()
{

}

void AlienBehaviour::run(float dt)
{
	refreshComponents(); //TODO: Find a way to not need to run this each tick
	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	if (!thisEnemy.life->statusCheck())
	{
		thisEnemy.entity->dead = true;
	}

	thisEnemy.ai->_debugState = state;

	thisEnemy.ai->_timer += dt;
	thisEnemy.ai->_attackTimer += dt;
	thisEnemy.ai->_stunTimer += dt;
	thisEnemy.ai->_newPathTimer += dt;

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > 50)
	{
		state = IDLE;
	}
	switch (state)
	{
	case IDLE:
		state = idleState(dt);
		break;
	case SEARCHING:
		state = searchingState(dt);
		break;
	case FOUND_GOAL:
		state = foundState(dt);
		break;
	case ATTACKING:
		state = attackingState(dt);
		break;
	}
	executeTransforms();
}

unsigned int AlienBehaviour::attackingState(float dt)
{
	//When the enemy attack, start the attack animation
	thisEnemy.drawObject->drawObject->mesh->setAnimationIndex(2);
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) >= thisEnemy.ai->_range)
	{
		thisEnemy.ai->_timer = 0;
		return SEARCHING;
	}
	else
	{
		std::mt19937 rng(thisEnemy.ai->rd());
		std::uniform_int_distribution<> randDmg(thisEnemy.ai->_damage - 1, thisEnemy.ai->_damage + 2);
		if (thisEnemy.ai->_attackTimer > 1.5)
		{
			//player->applyDamage(randDmg(rng));
			thisEnemy.ai->_attackTimer = 0;
		}

		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		thisEnemy.ai->_angle = atan2(playerDir.x, playerDir.z);
		thisEnemy.ai->_rotation = glm::angleAxis(thisEnemy.ai->_angle, glm::vec3(0, 1, 0));
	}
	return state;
}

RobotBehaviour::RobotBehaviour(std::shared_ptr<Hydra::World::Entity> enemy) : Behaviour(enemy)
{
	this->type = Type::ROBOT;
}

RobotBehaviour::RobotBehaviour()
{
	this->type = Type::ROBOT;
}

RobotBehaviour::~RobotBehaviour()
{

}

void RobotBehaviour::run(float dt)
{
	refreshComponents(); //TODO: Find a way to not need to run this each tick
	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	if (!thisEnemy.life->statusCheck())
	{
		thisEnemy.entity->dead = true;
	}

	thisEnemy.ai->_debugState = state;

	thisEnemy.ai->_timer += dt;
	thisEnemy.ai->_attackTimer += dt;
	thisEnemy.ai->_stunTimer += dt;
	thisEnemy.ai->_newPathTimer += dt;

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > 50)
	{
		state = IDLE;
	}
	switch (state)
	{
	case IDLE:
		state = idleState(dt);
		break;
	case SEARCHING:
		state = searchingState(dt);
		break;
	case FOUND_GOAL:
		state = foundState(dt);
		break;
	case ATTACKING:
		state = attackingState(dt);
		break;
	}
	executeTransforms();
}

unsigned int RobotBehaviour::attackingState(float dt)
{
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > thisEnemy.ai->_range)
	{
		thisEnemy.ai->_timer = 0;
		return SEARCHING;
	}
	else
	{
		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		thisEnemy.weapon->shoot(thisEnemy.transform->position, -playerDir, glm::quat(), 8.0f);
		thisEnemy.ai->_angle = atan2(playerDir.x, playerDir.z);
		thisEnemy.ai->_rotation = glm::angleAxis(thisEnemy.ai->_angle, glm::vec3(0, 1, 0));
	}
	return state;
}

AlienBossBehaviour::AlienBossBehaviour(std::shared_ptr<Hydra::World::Entity> enemy) : Behaviour(enemy)
{
	this->type = Type::ALIENBOSS;
}

AlienBossBehaviour::AlienBossBehaviour()
{
	this->type = Type::ALIENBOSS;
}

AlienBossBehaviour::~AlienBossBehaviour()
{

}

void AlienBossBehaviour::run(float dt)
{
	refreshComponents(); //TODO: Find a way to not need to run this each tick
	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	if (!thisEnemy.life->statusCheck())
	{
		thisEnemy.entity->dead = true;
	}

	thisEnemy.ai->_debugState = state;

	thisEnemy.ai->_timer += dt;
	thisEnemy.ai->_attackTimer += dt;
	thisEnemy.ai->_stunTimer += dt;
	thisEnemy.ai->_newPathTimer += dt;

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > 50)
	{
		state = IDLE;
	}
	switch (state)
	{
	case IDLE:
		state = idleState(dt);
		break;
	case SEARCHING:
		state = searchingState(dt);
		break;
	case FOUND_GOAL:
		state = foundState(dt);
		break;
	case ATTACKING:
		state = attackingState(dt);
		break;
	}
	executeTransforms();
}

unsigned int AlienBossBehaviour::attackingState(float dt)
{
	return IDLE;
}
