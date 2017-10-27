#pragma once
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/world/world.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hydra/component/componentmanager.hpp>
#include <hydra/ext/api.hpp>
#include <memory>

class HYDRA_PHYSICS_API Behaviour
{
public:
	Behaviour(std::shared_ptr<Hydra::World::Entity>& enemy);
	Behaviour();
	virtual ~Behaviour();

	enum class Type{ALIEN,ROBOT,ALIENBOSS};
	Type type = Type::ALIEN;

	enum {IDLE, SEARCHING, FOUND_GOAL, ATTACKING};
	unsigned int state = IDLE;

	virtual void run(float dt) = 0;
	void setEnemyEntity(std::shared_ptr<Hydra::World::Entity>& enemy);
	void setTargetPlayer(std::shared_ptr<Hydra::World::Entity>& player);
	void refreshComponents();
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
	};
	ComponentSet thisEnemy;
	ComponentSet targetPlayer;

	std::shared_ptr<PathFinding> pathFinding = std::make_shared<PathFinding>();

	virtual unsigned int idleState(float dt);
	virtual unsigned int searchingState(float dt);
	virtual unsigned int foundState(float dt);
	virtual unsigned int attackingState(float dt);

	virtual void executeTransforms();

};

class HYDRA_PHYSICS_API AlienBehaviour : public Behaviour
{
public:
	AlienBehaviour(std::shared_ptr<Hydra::World::Entity>& enemy);
	AlienBehaviour();
	~AlienBehaviour();
	void run(float dt);
	unsigned int attackingState(float dt);
private:

};

class HYDRA_PHYSICS_API RobotBehaviour : public Behaviour
{
public:
	RobotBehaviour(std::shared_ptr<Hydra::World::Entity>& enemy);
	RobotBehaviour();
	~RobotBehaviour();
	void run(float dt);
	unsigned int attackingState(float dt);
private:

};

class HYDRA_PHYSICS_API AlienBossBehaviour : public Behaviour
{
public:
	AlienBossBehaviour(std::shared_ptr<Hydra::World::Entity>& enemy);
	AlienBossBehaviour();
	~AlienBossBehaviour();
	void run(float dt);
	unsigned int attackingState(float dt);
private:

};