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

#include <btBulletDynamicsCommon.h>

Behaviour::Behaviour(std::shared_ptr<Hydra::World::Entity> enemy)
{

	thisEnemy.entity = enemy.get();
	refreshRequiredComponents();
	pathFinding = new PathFinding();

}
Behaviour::Behaviour()
{
	pathFinding = new PathFinding();
}

Behaviour::~Behaviour()
{
	delete pathFinding;
}

void Behaviour::setEnemyEntity(std::shared_ptr<Hydra::World::Entity> enemy)
{
	thisEnemy.entity = enemy.get();
	refreshRequiredComponents();
}

void Behaviour::setTargetPlayer(std::shared_ptr<Hydra::World::Entity> player)
{
	targetPlayer.entity = player.get();
	targetPlayer.life = player->getComponent<Hydra::Component::LifeComponent>().get();
	targetPlayer.transform = player->getComponent<Hydra::Component::TransformComponent>().get();
}
glm::vec2 Behaviour::flatVector(glm::vec3 vec)
{
	return glm::vec2(vec.x, vec.z);
}
void Behaviour::move(glm::vec3 target)
{
	glm::vec2 direction = glm::normalize(flatVector(target) - flatVector(thisEnemy.transform->position));

	thisEnemy.movement->velocity.x = (thisEnemy.movement->movementSpeed * direction.x);
	thisEnemy.movement->velocity.z = (thisEnemy.movement->movementSpeed * direction.y);

	rotation = glm::angleAxis(atan2(direction.x, direction.y), glm::vec3(0, 1, 0));
}
//Sets all components without setting new entities, use after adding new components to either entity
bool Behaviour::refreshRequiredComponents()
{
	hasRequiredComponents = (
		(thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>().get()) &&
		(thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>().get()) &&
		(thisEnemy.meshComp = thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>().get()) &&
		(thisEnemy.life = thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>().get()) &&
		(thisEnemy.rigidBody = thisEnemy.entity->getComponent<Hydra::Component::RigidBodyComponent>().get()) &&
		(targetPlayer.entity = thisEnemy.ai->getPlayerEntity().get()) &&
		(targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>().get())
		);
	return hasRequiredComponents;
}

unsigned int Behaviour::idleState(float dt)
{
	resetAnimationOnStart(0);
	//If the player is close enough, activate
	if (glm::distance(flatVector(thisEnemy.transform->position), flatVector(targetPlayer.transform->position)) < 50.0f)
	{
		return SEARCHING;
	}
	return state;
}

unsigned int Behaviour::searchingState(float dt)
{
	//While the enemy is searching, play the walking animation
	resetAnimationOnStart(1);
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) <= range)
	{
		pathFinding->foundGoal = true;
		return ATTACKING;
	}

	pathFinding->findPath(thisEnemy.transform->position, targetPlayer.transform->position);
	newPathTimer = 0;
	return MOVING;
}

unsigned int Behaviour::movingState(float dt)
{
	resetAnimationOnStart(1);

	float distEnemyToPlayer = glm::length(thisEnemy.transform->position - targetPlayer.transform->position);
	if (distEnemyToPlayer <= range)
	{
		isAtGoal = true;
		return ATTACKING;
	}

	//If enemy can see the player, move toward them
	if (pathFinding->inLineOfSight(thisEnemy.transform->position, targetPlayer.transform->position))
	{
		move(targetPlayer.transform->position);
	}
	//If there is nowhere to go, search (prob not needed, should always have a goal here)
	if (!pathFinding->pathToEnd.empty())
	{
		//Made these as the code got very hard to read otherwise
		float distEnemyToNextPos = glm::distance(flatVector(thisEnemy.transform->position), flatVector(pathFinding->pathToEnd.back()));
		float distEnemyToGoal = glm::distance(flatVector(thisEnemy.transform->position), flatVector(pathFinding->pathToEnd.front()));
		float distPlayerToGoal = glm::distance(flatVector(targetPlayer.transform->position), flatVector(pathFinding->pathToEnd.front()));

		//Check that the goal is closer to the player than we are, otherwise the path is invalid
		if (distPlayerToGoal < distEnemyToPlayer)
		{
			//If the next pos is reached move on
			if (distEnemyToNextPos <= 1.0f)
			{
				pathFinding->pathToEnd.pop_back();
				//If there is nowhere to go, search
				if (pathFinding->pathToEnd.empty())
				{
					move(targetPlayer.transform->position);
					return SEARCHING;
				}
				else
				{
					move(pathFinding->pathToEnd.back());
				}
			}
			else
			{
				move(pathFinding->pathToEnd.back());
			}
		}
		else
		{
			move(targetPlayer.transform->position);
			return SEARCHING;
		}
	}
	else
	{
		move(targetPlayer.transform->position);
		return SEARCHING;
	}

	if (newPathTimer >= newPathDelay)
	{
		newPathTimer = 0.0f;
		return SEARCHING;
	}
	return state;
}

unsigned int Behaviour::attackingState(float dt)
{
	//When the enemy attack, start the attack animation
	resetAnimationOnStart(2);
	if (glm::distance(thisEnemy.transform->position, targetPlayer.transform->position) >= range)
	{
		idleTimer = 0.0f;
		return SEARCHING;
	}
	else
	{
		std::mt19937 rng(rd());
		std::uniform_int_distribution<> randDmg(thisEnemy.ai->damage - 1, thisEnemy.ai->damage + 2);
		if (attackTimer > 2.5)
		{
			targetPlayer.life->applyDamage(randDmg(rng));
			attackTimer = 0;
		}

		glm::vec3 playerDir = glm::normalize(targetPlayer.transform->position - thisEnemy.transform->position);
		rotation = glm::angleAxis(atan2(playerDir.x, playerDir.z), glm::vec3(0, 1, 0));
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

	if(type != Type::BOSS_HAND)
		thisEnemy.transform->setRotation(rotation);
}

void Behaviour::resetAnimationOnStart(int animationIndex) {
	//When starting a new animation, use this to reset the keyframe to 0
	//This prevents animations to start in the middle of the animation
	//animationIndex is which animation should be switched to
	if (thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex != animationIndex) {
		thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->currentFrame = 0;
	}
	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = animationIndex;

}

void Behaviour::setPathMap(bool** map)
{
	pathFinding->map = map;
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
		if (!refreshRequiredComponents())
			return;
	if (thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>()->health <= 0)
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
	case MOVING:
		state = movingState(dt);
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
	resetAnimationOnStart(2);
	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) >= range)
	{
		idleTimer = 0;
		return SEARCHING;
	}
	else
	{
		std::mt19937 rng(rd());
		std::uniform_int_distribution<> randDmg(thisEnemy.ai->damage - 1, thisEnemy.ai->damage + 2);
		if (attackTimer > 2.5)
		{
			targetPlayer.life->applyDamage(randDmg(rng));
			attackTimer = 0;
		}

		glm::vec3 playerDir = glm::normalize(targetPlayer.transform->position - thisEnemy.transform->position);
		rotation = glm::angleAxis(atan2(playerDir.x, playerDir.z), glm::vec3(0, 1, 0));
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
	//thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->animationIndex = 0;
	if (!hasRequiredComponents)
		if (!RobotBehaviour::refreshRequiredComponents())
			return;
	if (thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>()->health <= 0)
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
	case MOVING:
		state = movingState(dt);
		break;
	case ATTACKING:
		state = attackingState(dt);
		break;
	}
	executeTransforms();
}

unsigned int RobotBehaviour::idleState(float dt)
{
	resetAnimationOnStart(0);
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
		(thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>().get()) &&
		(thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>().get()) &&
		(thisEnemy.meshComp = thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>().get()) &&
		(thisEnemy.weapon = thisEnemy.entity->getComponent<Hydra::Component::WeaponComponent>().get()) &&
		(thisEnemy.life = thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>().get()) &&
		(thisEnemy.rigidBody = thisEnemy.entity->getComponent<Hydra::Component::RigidBodyComponent>().get()) &&
		(targetPlayer.entity = thisEnemy.ai->getPlayerEntity().get()) &&
		(targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>().get())
		);
	return hasRequiredComponents;
}

unsigned int RobotBehaviour::attackingState(float dt)
{
	resetAnimationOnStart(2);

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > range)
	{
		idleTimer = 0;
		return SEARCHING;
	}
	else
	{
		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		thisEnemy.weapon->shoot(thisEnemy.transform->position + glm::vec3{ 0, 1.5, 0 }, playerDir, glm::quat(), 8.0f, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY_PROJECTILE);
		rotation = glm::angleAxis(atan2(playerDir.x, playerDir.z), glm::vec3(0, 1, 0));
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
	case MOVING:
		state = movingState(dt);
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
			thisEnemy.weapon->shoot(thisEnemy.transform->position, playerDir, glm::quat(), 15.0f, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY_PROJECTILE);
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
					//a->behaviour = std::make_shared<AlienBehaviour>(alienSpawn);
					a->damage = 4;
					//a->behaviour->originalRange = 4;
					a->radius = 2.0f;

					auto h = alienSpawn->addComponent<Hydra::Component::LifeComponent>();
					h->maxHP = 80;
					h->health = 80;

					auto m = alienSpawn->addComponent<Hydra::Component::MovementComponent>();
					m->movementSpeed = 8.0f;

					auto t = alienSpawn->addComponent<Hydra::Component::TransformComponent>();
					t->position = thisEnemy.transform->position + glm::vec3(3, thisEnemy.transform->position.y, 3);
					t->scale = glm::vec3{ 2,2,2 };

					alienSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel2.mATTIC");
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
			rotation = glm::angleAxis(atan2(playerDir.x, playerDir.z), glm::vec3(0, 1, 0));
		}
		return state;
	}
}


//BossHand_Left::BossHand_Left(std::shared_ptr<Hydra::World::Entity> enemy) :Behaviour(enemy)  {
//
//	this->type = Type::BOSS_LEFTHAND;
//
//}

BossHand_Left::BossHand_Left(std::shared_ptr<Hydra::World::Entity> enemy) : Behaviour(enemy) {

	this->type = Type::BOSS_HAND;

}

BossHand_Left::BossHand_Left() {

	this->type = Type::BOSS_HAND;

}

BossHand_Left::~BossHand_Left() {

}

void BossHand_Left::run(float dt) {

	if (!hasRequiredComponents)
		if (!refreshRequiredComponents())
			return;
	if (thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>()->health <= 0)
		return;

	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	thisEnemy.ai->debugState = state;

	idleTimer += dt;
	attackTimer += dt;
	newPathTimer += dt;

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > 500)
	{
		state = HandPhases::IDLEHAND;
	}

	switch (state)
	{
	case HandPhases::IDLEHAND:
		state = idleState(dt);
		break;
	case HandPhases::SWIPE:
		state = swipeState(dt);
		break;
	case HandPhases::SMASH:
		state = smashState(dt);
		break;
	case HandPhases::HANDCANON:
		state = canonState(dt);
		break;
	case HandPhases::COVER:
		state = coverState(dt);
		break;
	case HandPhases::RETURN:
		state = returnState(dt);
		break;
	}

	executeTransforms();

}

void BossHand_Left::move(glm::vec3 target) {
	
	glm::vec3 direction = glm::normalize(target - thisEnemy.transform->position);

	if (HandPhases::SMASH) {
		thisEnemy.movement->velocity.x = ((thisEnemy.movement->movementSpeed / 2) * direction.x);
		thisEnemy.movement->velocity.y = ((thisEnemy.movement->movementSpeed / 2) * direction.y);
		thisEnemy.movement->velocity.z = ((thisEnemy.movement->movementSpeed / 2) * direction.z);
	}
	else {
		thisEnemy.movement->velocity.x = (thisEnemy.movement->movementSpeed * direction.x);
		thisEnemy.movement->velocity.y = (thisEnemy.movement->movementSpeed * direction.y);
		thisEnemy.movement->velocity.z = (thisEnemy.movement->movementSpeed * direction.z);
	}
	//rotation = glm::angleAxis(atan2(direction.x, direction.y), glm::vec3(0, 1, 0));

}
void BossHand_Left::rotateAroundAxis(float newRotation, glm::vec3(direction)) {

	rotation = glm::angleAxis(glm::radians(newRotation), direction);

}

unsigned int BossHand_Left::idleState(float dt) {

	int state = HandPhases::IDLEHAND;
	idleTimer += dt;
	//Wait 2 seconds before next move
	if (idleTimer >= 5.0f) {
		int randomNextMove = rand() % 125;

		if (randomNextMove < 30) {
			Hydra::IEngine::getInstance()->log(Hydra::LogLevel::normal, "Boss Smash");
			return HandPhases::SMASH;
		}
		else if (randomNextMove < 60) {
			Hydra::IEngine::getInstance()->log(Hydra::LogLevel::normal, "Boss Swipe");
			return HandPhases::SWIPE;
		}
		else if (randomNextMove < 85) {
			Hydra::IEngine::getInstance()->log(Hydra::LogLevel::normal, "Boss Canon");
			return HandPhases::HANDCANON;
		}
		else if (randomNextMove < 100) {
			Hydra::IEngine::getInstance()->log(Hydra::LogLevel::normal, "Boss Cover");
			return HandPhases::COVER;
		}
		else
			return HandPhases::IDLEHAND;
		idleTimer = 0.0f;
	}
	return state;
}

unsigned int BossHand_Left::smashState(float dt) {

	int state = HandPhases::SMASH;
	if (smashing == false) {
		move(glm::vec3(targetPlayer.transform->position.x, originalHeight, targetPlayer.transform->position.z));
		if (glm::distance(flatVector(thisEnemy.transform->position), flatVector(targetPlayer.transform->position)) < 1.0f) {
			smashing = true;
			smashPosition = glm::vec3(targetPlayer.transform->position.x, 0, targetPlayer.transform->position.z);
		}
	}

	if (smashing == true) {
		waitToSmashTimer += dt;
		if (waitToSmashTimer >= 1.0f)
			move(smashPosition);
		if (hit == false) {
			if (glm::distance(thisEnemy.transform->position, targetPlayer.transform->position) < 5.1f) {
				targetPlayer.life->applyDamage(10);
				hit = true;
			}
		}
		if (thisEnemy.transform->position.y <= 3.5f) {
			state = HandPhases::RETURN;
			thisEnemy.transform->position.y = originalHeight;
			smashing = false;
			waitToSmashTimer = 0.0f;
			hit = false;
		}
	}

	return state;
}

unsigned int BossHand_Left::swipeState(float dt) {
	
	int state = HandPhases::SWIPE;
	
	if (swiping == false) {
		swipePosition.x = targetPlayer.transform->position.x;
		move(swipePosition);
	}
	if (thisEnemy.transform->position.x >= targetPlayer.transform->position.x - 1 && thisEnemy.transform->position.x <= targetPlayer.transform->position.x + 1) {
		swiping = true;
	}
	if (hit == false) {
		if (glm::distance(thisEnemy.transform->position, targetPlayer.transform->position) < 3.0f) {
			targetPlayer.life->applyDamage(10);
			hit = true;
		}
	}
	if (swiping == true) {
		move(glm::vec3(swipePosition.x, swipePosition.y, -swipePosition.z));
		//thisEnemy.transform->position.z -= 0.1f;
		if (glm::distance(flatVector(thisEnemy.transform->position), flatVector(glm::vec3(swipePosition.x, swipePosition.y, -swipePosition.z))) < 2.0f) {
			state = HandPhases::RETURN;
			swiping = false;
			hit = false;			
		}
	}

	return state;
}

unsigned int BossHand_Left::canonState(float dt) {

	int state = HandPhases::HANDCANON;
	resetAnimationOnStart(2);
	if (!shooting) {
		randomNrOfShots = rand() % 60 + 40;
		move(canonPosition);
	}
	//Get into shooting position

	if (glm::distance(flatVector(thisEnemy.transform->position), flatVector(canonPosition)) < 1.0f) {
		shooting = true;
		glm::vec3 playerDir = targetPlayer.transform->position - thisEnemy.transform->position;
		playerDir = glm::normalize(playerDir);
		glm::vec3 shootPosition = thisEnemy.transform->position + glm::vec3{ 0 , 1.5, 0 };

		thisEnemy.weapon->shoot(shootPosition, playerDir, glm::quat(), 2.2,
			Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY_PROJECTILE);
		rotation = glm::angleAxis(atan2(playerDir.x, playerDir.z), glm::vec3(0, 1, 0));
		shotsFired++;
		if (shotsFired >= randomNrOfShots) {
			shotsFired = 0;
			shooting = false;
			state = HandPhases::RETURN;
		}
	}

	return state;
}

unsigned int BossHand_Left::coverState(float dt) {

	int state = HandPhases::COVER;
	bool covering = false;
	resetAnimationOnStart(1);

	if (glm::distance(flatVector(thisEnemy.transform->position), flatVector(coverPosition)) < 1.0f) {
		covering = true;
		coverTimer += dt;
	}
	else
		move(coverPosition);

	if (coverTimer >= 5){
		coverTimer = 0;
		state = HandPhases::RETURN;
	}

	return state;
}

unsigned int BossHand_Left::returnState(float dt)
{
	int state = HandPhases::RETURN;
	resetAnimationOnStart(0);

	move(basePosition);
	//rotateAroundAxis(-90, glm::vec3(1, 0, 0));

	if (glm::distance(flatVector(thisEnemy.transform->position), flatVector(basePosition)) < 3.0f)
		state = HandPhases::IDLEHAND;

	return state;
}

bool BossHand_Left::refreshRequiredComponents()
{
	hasRequiredComponents = (
		(thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>().get()) &&
		(thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>().get()) &&
		(thisEnemy.meshComp = thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>().get()) &&
		(thisEnemy.weapon = thisEnemy.entity->getComponent<Hydra::Component::WeaponComponent>().get()) &&
		(thisEnemy.life = thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>().get()) &&
		(thisEnemy.rigidBody = thisEnemy.entity->getComponent<Hydra::Component::RigidBodyComponent>().get()) &&
		(targetPlayer.entity = thisEnemy.ai->getPlayerEntity().get()) &&
		(targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>().get())
		);
	return hasRequiredComponents;
}



BossArm::BossArm(std::shared_ptr<Hydra::World::Entity> enemy) : Behaviour(enemy) {
	this->type = Type::BOSS_ARMS;
}

BossArm::BossArm() {
	this->type = Type::BOSS_ARMS;
	
}

BossArm::~BossArm() {
}

void BossArm::run(float dt) {

	if (!hasRequiredComponents)
		if (!refreshRequiredComponents())
			return;

	thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	thisEnemy.ai->debugState = state;

	idleTimer += dt;
	attackTimer += dt;
	newPathTimer += dt;

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > 500)
	{
		state = ArmPhases::CHILL;
	}

	thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>()->mesh->getAnimationCounter();

	switch (state)
	{
	case ArmPhases::CHILL:
		state = idleState(dt);
		break;
	case  ArmPhases::SINGLE_BONK:
		state = bonkState(dt);
		break;
	case  ArmPhases::MULTIPLE_BONK:
		state = multipleBonkState(dt);
		break;
	case  ArmPhases::SWEEP:
		state = sweepState(dt);
		break;

	}
	updateRigidBodyPosition();
	executeTransforms();

}

unsigned int BossArm::idleState(float dt) {
	int state = ArmPhases::CHILL;


	return state;
}

unsigned int BossArm::bonkState(float dt) {
	int state = ArmPhases::SINGLE_BONK;


	return state;
}

unsigned int BossArm::multipleBonkState(float dt) {
	int state = ArmPhases::MULTIPLE_BONK;


	return state;
}

unsigned int BossArm::sweepState(float dt)
{
	int state = ArmPhases::SWEEP;


	return state;
}

void BossArm::updateRigidBodyPosition() {

	//thisEnemy.rigidBody->

}

bool BossArm::refreshRequiredComponents() {
	hasRequiredComponents = (
		(thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>().get()) &&
		(thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>().get()) &&
		(thisEnemy.meshComp = thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>().get()) &&
		(thisEnemy.movement = thisEnemy.entity->getComponent<Hydra::Component::MovementComponent>().get()) &&
		(thisEnemy.rigidBody = thisEnemy.entity->getComponent<Hydra::Component::RigidBodyComponent>().get()) &&
		(targetPlayer.entity = thisEnemy.ai->getPlayerEntity().get()) &&
		(targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>().get())
		);
	return hasRequiredComponents;
}

StationaryBoss::StationaryBoss(std::shared_ptr<Hydra::World::Entity> enemy) :Behaviour(enemy){
	this->type = Type::STATINARY_BOSS;
	applySpawnPositions();
}

StationaryBoss::StationaryBoss() {
	this->type = Type::STATINARY_BOSS;
	applySpawnPositions();
}

StationaryBoss::~StationaryBoss() {
}

void StationaryBoss::run(float dt) {
	
	if (!hasRequiredComponents)
		if (!refreshRequiredComponents())
			return;
	if (thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>()->health <= 0)
		return;

	//thisEnemy.movement->velocity = glm::vec3(0, 0, 0);
	thisEnemy.ai->debugState = state;

	idleTimer += dt;
	attackTimer += dt;
	newPathTimer += dt;

	if (glm::length(thisEnemy.transform->position - targetPlayer.transform->position) > 500)
	{
		state = StatinoaryBossPhases::NOTHING;
	}

	switch (state)
	{
	case StatinoaryBossPhases::NOTHING:
		state = idleState(dt);
		break;
	case StatinoaryBossPhases::SPAWN:
		state = spawnState(dt);
		break;
	}

	//executeTransforms();
}

unsigned int StationaryBoss::idleState(float dt) {

	int state = StatinoaryBossPhases::NOTHING;

	if (thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>()->health <= spawnEnemiesAtPercentage[spawnIndex]) {
		state = StatinoaryBossPhases::SPAWN;
		spawnIndex++;
	}

	return state;
	
}
unsigned int StationaryBoss::spawnState(float dt) {

	int state = StatinoaryBossPhases::NOTHING;
	
	randomAliens = rand() % maxSpawn;
	randomRobots = maxSpawn - randomAliens;
	for (int i = 0; i < randomAliens; i++) {
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
		t->position = spawnPositions[rand() % spawnPositions.size()];
		t->scale = glm::vec3{ 2,2,2 };

		alienSpawn->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienModel2.mATTIC");
	}
	for (int i = 0; i < randomAliens; i++) {
		auto alienEntity = world::newEntity("FastAlien1", world::root());
		alienEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh("assets/objects/characters/AlienFastModel2.mATTIC");
		auto a = alienEntity->addComponent<Hydra::Component::AIComponent>();
		a->behaviour = std::make_shared<AlienBehaviour>(alienEntity);

		auto h = alienEntity->addComponent<Hydra::Component::LifeComponent>();
		h->maxHP = 60;
		h->health = 60;

		auto m = alienEntity->addComponent<Hydra::Component::MovementComponent>();
		m->movementSpeed = 10.0f;

		auto t = alienEntity->addComponent<Hydra::Component::TransformComponent>();
		t->position = spawnPositions[rand() % spawnPositions.size()];
		t->scale = glm::vec3{ 1,1,1 };

		auto rgbc = alienEntity->addComponent<Hydra::Component::RigidBodyComponent>();
		rgbc->createBox(glm::vec3(0.5f, 1.5f, 0.5f) * t->scale, glm::vec3(0, 1.5, 0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_ENEMY, 100.0f,
			0, 0, 0.6f, 1.0f);
		rgbc->setActivationState(Hydra::Component::RigidBodyComponent::ActivationState::disableDeactivation);
		rgbc->setAngularForce(glm::vec3(0));
	}
	maxSpawn *= 2;
	return state;

}

//unsigned int StationaryBoss::shootingState(float dt) {
//
//
//	
//}

void StationaryBoss::applySpawnPositions() {

	for (int i = -18; i < 60; i += 5) {
		spawnPositions.push_back(glm::vec3(-34 , 3, i));
	}
	for (int i = -30; i < 30; i +=3) {
		spawnPositions.push_back(glm::vec3(i, 3, -20));
	}
	for (int i = -12; i < 60; i += 5) {
		spawnPositions.push_back(glm::vec3(34, 3, i));
	}
	//for (int i = -55; i < 60; i += 5) {
	//	spawnPositions.push_back(glm::vec3(i, 3, -40 - (i - 4)));
	//}
}

bool StationaryBoss::refreshRequiredComponents()
{
	hasRequiredComponents = (
		(thisEnemy.ai = thisEnemy.entity->getComponent<Hydra::Component::AIComponent>().get()) &&
		(thisEnemy.transform = thisEnemy.entity->getComponent<Hydra::Component::TransformComponent>().get()) &&
		(thisEnemy.meshComp = thisEnemy.entity->getComponent<Hydra::Component::MeshComponent>().get()) &&
		//(thisEnemy.weapon = thisEnemy.entity->getComponent<Hydra::Component::WeaponComponent>().get()) &&
		(thisEnemy.life = thisEnemy.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(thisEnemy.rigidBody = thisEnemy.entity->getComponent<Hydra::Component::RigidBodyComponent>().get()) &&
		(targetPlayer.entity = thisEnemy.ai->getPlayerEntity().get()) &&
		(targetPlayer.life = targetPlayer.entity->getComponent<Hydra::Component::LifeComponent>().get()) &&
		(targetPlayer.transform = targetPlayer.entity->getComponent<Hydra::Component::TransformComponent>().get())
		);
	return hasRequiredComponents;
}
