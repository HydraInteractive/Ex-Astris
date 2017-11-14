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
#include <glm/gtx/transform.hpp>

Behaviour::Behaviour(std::shared_ptr<Hydra::World::Entity> enemy)
{

	thisEnemy.entity = enemy;
	refreshRequiredComponents();
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
	refreshRequiredComponents();
}

void Behaviour::setTargetPlayer(std::shared_ptr<Hydra::World::Entity> player)
{
	targetPlayer.entity = player;
	targetPlayer.life = player->getComponent<Hydra::Component::LifeComponent>();
	targetPlayer.transform = player->getComponent<Hydra::Component::TransformComponent>();
}
//Sets all components without setting new entities, use after adding new components to either entity
bool Behaviour::refreshRequiredComponents()
{
	hasRequiredComponents = (
		(thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>()) &&
		(thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>()) &&
		(thisEnemy.drawObject = thisEnemy.entity->getComponent<Hydra::Component::DrawObjectComponent>()) &&
		(thisEnemy.life = thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>()) &&
		(thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>()) && 
		(thisEnemy.rigidBody = thisEnemy.entity->getComponent<Hydra::Component::RigidBodyComponent>()) &&
		(thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>()) &&
		(targetPlayer.entity = thisEnemy.ai->getPlayerEntity()) &&
		(targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>()) &&
		(targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>())
	 );
	return hasRequiredComponents;
}

bool Behaviour::checkLOS(int levelmap[MAP_SIZE][MAP_SIZE], glm::vec3 enemyPos, glm::vec3 playerPos)
{
	//TODO: ADD COORDINATE VERIFICATION, MAYBE MOVE TO PATHFINDING
	//New code, not optimal
	double x = playerPos.x - enemyPos.x;
	double z = playerPos.z - enemyPos.z;
	double len = std::sqrt((x*x) + (z*z));

	if (!len) //Player tile is same as target tile
		return true;

	double unitx = x / len;
	double unitz = z / len;

	x = enemyPos.x;
	z = enemyPos.z;
	for (double i = 1; i < len; i += 1)
	{
		if (levelmap[(int)x][(int)z] == 1)
			{
				return false;
			}
		x += unitx;
		z += unitz;
	}
	return true;
}

unsigned int Behaviour::idleState(float dt)
{
	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = 0;
	//Play the idle animation
	//if (targetPlayer.transform->position.x > mapOffset.x && targetPlayer.transform->position.x < MAP_SIZE && targetPlayer.transform->position.z > mapOffset.z && targetPlayer.transform->position.z < MAP_SIZE)
	//{
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) < 50)
	{
		idleTimer = 0;
		return SEARCHING;

	}
	//}
	return state;
}

unsigned int Behaviour::searchingState(float dt)
{
	//While the enemy is searching, play the walking animation
	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = 1;
	if (idleTimer >= 5)
	{
		return IDLE;
	}
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) < range)
	{
		isAtGoal = true;
		pathFinding->foundGoal = true;
		return ATTACKING;
	}

	//if (targetPlayer.transform->position.x <= mapOffset.x || targetPlayer.transform->position.x >= MAP_SIZE || targetPlayer.transform->position.z <= mapOffset.z || targetPlayer.transform->position.z >= MAP_SIZE)
	//{
	//	return IDLE;
	//}
	pathFinding->intializedStartGoal = false;
	pathFinding->findPath(thisEnemy.transform->position, targetPlayer.transform->position, map);
	isAtGoal = false;


	if (pathFinding->foundGoal)
	{
		if (!pathFinding->_pathToEnd.empty())
		{
			targetPos = pathFinding->_pathToEnd[0];
		}
		newPathTimer = 0;
		return FOUND_GOAL;
	}
	return state;
}

unsigned int Behaviour::foundState(float dt)
{
	if (!isAtGoal)
	{
		if (!pathFinding->_pathToEnd.empty())
		{
			glm::vec3 targetDistance = pathFinding->nextPathPos(thisEnemy.transform->position, thisEnemy.ai->radius) - thisEnemy.transform->position;

			angle = atan2(targetDistance.x, targetDistance.z);
			rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));

			glm::vec3 direction = glm::normalize(targetDistance);

			thisEnemy.movement->velocity.x = (thisEnemy.movement->movementSpeed * direction.x);
			thisEnemy.movement->velocity.z = (thisEnemy.movement->movementSpeed * direction.z);
			if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) <= range)
			{
				isAtGoal = true;
				pathFinding->foundGoal = true;
				return ATTACKING;
			}
			else if (glm::length(thisEnemy.transform->position - targetPos) <= 3.0f)
			{
				idleTimer = 0;
				return SEARCHING;
			}
			else if (glm::length(targetPlayer.transform->position.x - targetPos.x) > 25.0f || glm::length(targetPlayer.transform->position.z - targetPos.z) > 25.0f)
			{
				idleTimer = 0;
				return SEARCHING;
			}
		}
	}

	if (newPathTimer >= 4.0f)
	{
		idleTimer = 0.0f;
		return SEARCHING;
	}

	//if (targetPlayer.transform->position.x <= mapOffset.x || targetPlayer.transform->position.x >= MAP_SIZE || targetPlayer.transform->position.z <= mapOffset.z || targetPlayer.transform->position.z >= MAP_SIZE)
	//{
	//	return IDLE;
	//}
	return state;
}

unsigned int Behaviour::attackingState(float dt)
{
	//When the enemy attack, start the attack animation
	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = 2;
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) >= range)
	{
		idleTimer = 0.0f;
		return SEARCHING;
	}
	else
	{
		std::mt19937 rng(rd());
		std::uniform_int_distribution<> randDmg(thisEnemy.ai->damage - 1, thisEnemy.ai->damage + 2);
		if (attackTimer > 1.5f)
		{
			//player->applyDamage(randDmg(rng));
			attackTimer = 0.0f;
		}

		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		angle = atan2(playerDir.x, playerDir.z);
		rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
	}
	return state;
}

void Behaviour::executeTransforms()
{
	range = originalRange;

	auto rigidBody = static_cast<btRigidBody*>(thisEnemy.rigidBody->getRigidBody());
	glm::vec3 movementForce = thisEnemy.movement->velocity;
	//if (movementForce.x = 0 && movementForce.y == 0 && movementForce.z == 0)
	//	rigidBody->clearForces();
	//else
	rigidBody->setLinearVelocity(btVector3(movementForce.x, movementForce.y, movementForce.z));
	
	thisEnemy.transform->setRotation(rotation);
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
	//If all components haven't been found, try to find them and abort if one or more do not exist
	if (!hasRequiredComponents)
		if(!refreshRequiredComponents())
			return;
	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	thisEnemy.ai->debugState = state;

	idleTimer += dt;
	attackTimer += dt;
	newPathTimer += dt;

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
	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = 2;
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) >= range)
	{
		idleTimer = 0;
		return SEARCHING;
	}
	else
	{
		std::mt19937 rng(rd());
		std::uniform_int_distribution<> randDmg(thisEnemy.ai->damage - 1, thisEnemy.ai->damage + 2);
		if (attackTimer > 1.5f)
		{
			targetPlayer.life->applyDamage(randDmg(rng));
			attackTimer = 0;
		}

		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		angle = atan2(playerDir.x, playerDir.z);
		rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
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
	//If all components haven't been found, try to find them and abort if one or more do not exist
	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = 0;
	if (!hasRequiredComponents)
		if (!refreshRequiredComponents())
			return;
	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	// Same as above.
	//if (!thisEnemy.life->statusCheck())
	//{
	//	thisEnemy.entity->dead = true;
	//}

	thisEnemy.ai->debugState = state;

	idleTimer += dt;
	attackTimer += dt;
	newPathTimer += dt;

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

unsigned int RobotBehaviour::idleState(float dt)
{
	if (thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex != 0) {
		thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->currentFrame = 0;
	}
	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = 0;
	int currentFrame = thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->currentFrame;
	if (currentFrame > 50) {
		thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->currentFrame = 50;
	}

	//if (targetPlayer.transform->position.x > mapOffset.x && targetPlayer.transform->position.x < MAP_SIZE && targetPlayer.transform->position.z > mapOffset.z && targetPlayer.transform->position.z < MAP_SIZE)
	{
		if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) < 50)
		{
			idleTimer = 0;
			if (currentFrame >= 2)
				thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->currentFrame -= 2;
			else
				return SEARCHING;
			}
		}
	return state;
}

bool RobotBehaviour::refreshRequiredComponents()
{
	hasRequiredComponents = (
		(thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>()) &&
		(thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>()) &&
		(thisEnemy.drawObject = thisEnemy.entity->getComponent<Hydra::Component::DrawObjectComponent>()) &&
		(thisEnemy.weapon = thisEnemy.entity->getComponent<Hydra::Component::WeaponComponent>()) &&
		(thisEnemy.life = thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>()) &&
		(thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>()) &&
		(targetPlayer.entity = thisEnemy.ai->getPlayerEntity()) &&
		(targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>()) &&
		(targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>())
	);
	return hasRequiredComponents;
}

unsigned int RobotBehaviour::attackingState(float dt)
{
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > range)
	{
		idleTimer = 0;
		return SEARCHING;
	}
	else
	{
		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		thisEnemy.weapon->shoot(thisEnemy.transform->position, -playerDir, glm::quat(), 8.0f);
		angle = atan2(playerDir.x, playerDir.z);
		rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
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
	//If all components haven't been found, try to find them and abort if one or more do not exist
	if (!hasRequiredComponents)
		if (!refreshRequiredComponents())
			return;
	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	if (!thisEnemy.life->statusCheck())
	{
		thisEnemy.entity->dead = true;
	}

	thisEnemy.ai->debugState = state;

	idleTimer += dt;
	attackTimer += dt;
	stunTimer += dt;
	newPathTimer += dt;
	phaseTimer += dt;
	spawnTimer += dt;

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
		phaseTimer = 0;
		break;
	case FOUND_GOAL:
		state = foundState(dt);
		phaseTimer = 0;
		break;
	case ATTACKING:
		state = attackingState(dt);
		break;
	}
	executeTransforms();
}

unsigned int AlienBossBehaviour::attackingState(float dt)
{
	using world = Hydra::World::World;

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > range && !stunned)
	{
		idleTimer = 0;
		return SEARCHING;
	}
	else
	{
		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);

		switch (bossPhase)
		{
		case BossPhase::CLAWING:
		{
			originalRange = 9.0f;
			thisEnemy.movement->movementSpeed = 3.0f;
			std::mt19937 rng(rd());
			std::uniform_int_distribution<> randDmg(thisEnemy.ai->damage - 1, thisEnemy.ai->damage + 2);
			if (attackTimer >= 3)
			{
				targetPlayer.life->applyDamage(randDmg(rng));
				attackTimer = 0;
			}

			if (phaseTimer >= 10)
			{
				bossPhase = SPITTING;
				phaseTimer = 0;
			}
		}break;
		case BossPhase::SPITTING:
		{
			range = 30.0f;
			thisEnemy.weapon->shoot(thisEnemy.transform->position, -playerDir, glm::quat(), 15.0f);
			if (phaseTimer >= 10)
			{
				bossPhase = SPAWNING;
				phaseTimer = 0;
			}
		}break;
		case BossPhase::SPAWNING:
		{
			range = 30.0f;
			if (spawnAmount <= 3)
			{
				if (spawnTimer >= 2)
				{
					auto alienSpawn = world::newEntity("AlienSpawn", world::root());

					auto a = alienSpawn->addComponent <Hydra::Component::AIComponent>();
					a->behaviour = std::make_shared<AlienBehaviour>(alienSpawn);
					a->damage = 4;
					a->behaviour->originalRange = 4;
					a->radius = 2.0f;

					auto h = alienSpawn->addComponent<Hydra::Component::LifeComponent>();
					h->maxHP = 80;
					h->health = 80;

					auto m = alienSpawn->addComponent<Hydra::Component::MovementComponent>();
					m->movementSpeed = 8.0f;

					auto t = alienSpawn->addComponent<Hydra::Component::TransformComponent>();
					t->position = thisEnemy.transform->position + glm::vec3(3, thisEnemy.transform->position.y, 3);
					t->scale = glm::vec3{ 2,2,2 };

					alienSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel.mATTIC");
					spawnAmount++;
					spawnTimer = 0;
				}
			}

			if (phaseTimer >= 10)
			{
				bossPhase = CHILLING;
				phaseTimer = 0;
				stunTimer = 0;
			}
		}break;
		case BossPhase::CHILLING:
		{
			range = 30.0f;
			if (stunTimer >= 10)
			{
				stunned = !stunned;
				if (stunned)
				{
					bossPhase = CLAWING;
				}
			}
		}break;
		}

		if (!stunned)
		{
			angle = atan2(playerDir.x, playerDir.z);
			rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
		}
		return state;
	}
}
