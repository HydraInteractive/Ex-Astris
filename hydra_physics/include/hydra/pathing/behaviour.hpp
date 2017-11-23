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

	enum class Type { ALIEN, ROBOT, ALIENBOSS };
	Type type = Type::ALIEN;

	enum { IDLE, SEARCHING, MOVING, ATTACKING };
	unsigned int state = IDLE;

	enum BossPhase { CLAWING, SPITTING, SPAWNING, CHILLING };
	BossPhase bossPhase = BossPhase::CLAWING;

	float idleTimer = 0.0f;
	float attackTimer = 0.0f;
	float newPathTimer = 0.0f;
	float newPathDelay = 1.0f;
	float spawnTimer = 0.0f;
	float phaseTimer = 0.0f;

	std::random_device rd;
	bool playerSeen = false;
	bool isAtGoal = false;
	int oldMapPosX = 0;
	int oldMapPosZ = 0;

	bool hasRequiredComponents = false;
	bool playerUnreachable = false;

	float range = 1.0f;
	float originalRange = 1.0f;
	glm::quat rotation = glm::quat();

	virtual void run(float dt) = 0;
	void setEnemyEntity(std::shared_ptr<Hydra::World::Entity> enemy);
	void setTargetPlayer(std::shared_ptr<Hydra::World::Entity> player);
	virtual void setPathMap(bool** map);
protected:
	struct ComponentSet
	{
		Hydra::World::Entity* entity = nullptr;
		Hydra::Component::TransformComponent* transform = nullptr;
		Hydra::Component::MeshComponent* meshComp = nullptr;
		Hydra::Component::WeaponComponent* weapon = nullptr;
		Hydra::Component::LifeComponent* life = nullptr;
		Hydra::Component::MovementComponent* movement = nullptr;
		Hydra::Component::AIComponent* ai = nullptr;
		Hydra::Component::RigidBodyComponent* rigidBody = nullptr;
	};
	ComponentSet thisEnemy = ComponentSet();
	ComponentSet targetPlayer = ComponentSet();

	PathFinding* pathFinding = nullptr;

	glm::vec2 flatVector(glm::vec3 vec);
	void move(glm::vec3 target);
	virtual bool refreshRequiredComponents();
	virtual unsigned int idleState(float dt);
	virtual unsigned int searchingState(float dt);
	virtual unsigned int movingState(float dt);
	virtual unsigned int attackingState(float dt);
	virtual void executeTransforms();
	virtual void resetAnimationOnStart(int animationIndex);

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
	unsigned int idleState(float dt) final;
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

	float stunTimer = 0.0f;
	bool stunned = false;
	int spawnAmount = 0;

	void run(float dt);
	unsigned int attackingState(float dt) final;
private:
};
