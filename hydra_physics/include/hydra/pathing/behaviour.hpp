#pragma once
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/world/world.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hydra/component/componentmanager.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/ext/api.hpp>
#include <memory>
#include <random>

class HYDRA_PHYSICS_API Behaviour
{
public:
	Behaviour(std::shared_ptr<Hydra::World::Entity> enemy);
	Behaviour();
	virtual ~Behaviour();

	enum class Type{ALIEN,ROBOT,ALIENBOSS};
	Type type = Type::ALIEN;

	enum {IDLE, SEARCHING, FOUND_GOAL, ATTACKING};
	unsigned int state = IDLE;

	enum BossPhase {CLAWING, SPITTING, SPAWNING, CHILLING};
	BossPhase bossPhase = BossPhase::CLAWING;

	float idleTimer = 0;
	float attackTimer = 0;
	float newPathTimer = 0;
	float spawnTimer = 0;
	float phaseTimer = 0;

	std::random_device rd;
	int map[MAP_SIZE][MAP_SIZE];
	bool playerSeen = false;
	bool isAtGoal = false;
	int oldMapPosX = 0;
	int oldMapPosZ = 0;
	float angle = 1;

	bool hasRequiredComponents = false;

	float range = 1;
	float originalRange = 1;
	glm::vec3 mapOffset = glm::vec3(-30.0f, 0, -30.0f);
	glm::vec3 targetPos = glm::vec3{ 0, 0, 0 };
	glm::quat rotation = glm::quat();

	virtual void run(float dt) = 0;
	void setEnemyEntity(std::shared_ptr<Hydra::World::Entity> enemy);
	void setTargetPlayer(std::shared_ptr<Hydra::World::Entity> player);

	bool checkLOS(int levelmap[MAP_SIZE][MAP_SIZE], glm::vec3 A, glm::vec3 B);
protected:
	struct ComponentSet
	{
		std::shared_ptr<Hydra::World::Entity> entity;
		std::shared_ptr<Hydra::Component::TransformComponent> transform;
		std::shared_ptr<Hydra::Component::DrawObjectComponent> drawObject;
		std::shared_ptr<Hydra::Component::WeaponComponent> weapon;
		std::shared_ptr<Hydra::Component::LifeComponent> life;
		std::shared_ptr<Hydra::Component::MovementComponent> movement;
		std::shared_ptr<Hydra::Component::AIComponent> ai;
		std::shared_ptr<Hydra::Component::RigidBodyComponent> rigidBody;
	};
	ComponentSet thisEnemy;
	ComponentSet targetPlayer;

	std::shared_ptr<PathFinding> pathFinding = std::make_shared<PathFinding>();

	virtual bool refreshRequiredComponents();
	virtual unsigned int idleState(float dt);
	virtual unsigned int searchingState(float dt);
	virtual unsigned int foundState(float dt);
	virtual unsigned int attackingState(float dt);
	virtual void executeTransforms();
};

class HYDRA_PHYSICS_API AlienBehaviour final : public Behaviour
{
public:
	AlienBehaviour(std::shared_ptr<Hydra::World::Entity> enemy);
	AlienBehaviour();
	~AlienBehaviour();
	void run(float dt);

	unsigned int attackingState(float dt) final;
};

class HYDRA_PHYSICS_API RobotBehaviour final : public Behaviour
{
public:
	RobotBehaviour(std::shared_ptr<Hydra::World::Entity> enemy);
	RobotBehaviour();
	~RobotBehaviour();
	void run(float dt);

	unsigned int attackingState(float dt) final;
private:
	bool refreshRequiredComponents() final;
};

class HYDRA_PHYSICS_API AlienBossBehaviour final : public Behaviour
{
public:
	AlienBossBehaviour(std::shared_ptr<Hydra::World::Entity> enemy);
	AlienBossBehaviour();
	~AlienBossBehaviour();

	float stunTimer = 0;
	bool stunned = false;
	int spawnAmount = 0;

	void run(float dt);
	unsigned int attackingState(float dt) final;
private:
};
