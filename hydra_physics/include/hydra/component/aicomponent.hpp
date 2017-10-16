/**
* EnemyComponent/AI.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once
#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/component/playercomponent.hpp>
#include <math.h>
#include <SDL2/SDL.h>
#include <random>

using namespace Hydra::World;
namespace Hydra::Component {

	enum class EnemyTypes {
		Alien = 0,
		Robot = 1,
		AlienBoss = 2,
		AlienSpawner = 3,
		RobotSpawner = 4,
		RobotBoss = 5,
	};

	enum PathState{
		IDLE = 0,
		SEARCHING = 1,
		FOUND_GOAL = 2,
		ATTACKING = 3,
	};

	enum BossPhase {
		CLAWING = 0,
		SPITTING = 1,
		SPAWNING = 2,
		CHILLING = 3
	};

	class HYDRA_API EnemyComponent final : public IComponent{
	public:
		EnemyComponent(IEntity* entity);
		EnemyComponent(IEntity* entity, EnemyTypes enemyID, glm::vec3 pos, int hp, int dmg, float range, glm::vec3 scale);
		~EnemyComponent() final;

		void tick(TickAction action, float delta) final;
		// If you want to add more than one TickAction, combine them with '|' (The bitwise or operator) 
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "EnemyComponent"; }

		glm::vec3 getPosition();
		float getRadius();
		std::shared_ptr<Hydra::World::IEntity> getPlayerComponent();

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
		int getWall(int x, int y);
	private:
		PathState _pathState;
		PathFinding* _pathFinding = new PathFinding();
		BossPhase _bossPhase;
		std::vector<std::shared_ptr<Hydra::World::IEntity>> _spawnGroup;

		float _angle;
		float _range;
		float _originalRange;

		int _debugState;
		int _spawnAmount;
		int _health;
		int _damage;

		glm::vec3 _velocity;
		glm::vec3 _mapOffset;
		glm::vec3 _targetPos;
		glm::vec3 _position;
		glm::vec3 _startPosition;
		glm::vec3 _scale;

		glm::quat _rotation;

		bool _isAtGoal;
		bool _falling;
		bool _patrolPointReached;
		bool _playerSeen;
		bool _stunned;

		EnemyTypes _enemyID = EnemyTypes::Alien;
		std::random_device rd;
		Uint32 _timer;
		Uint32 _spawnTimer;
		Uint32 _stunTimer;
		Uint32 _attackTimer;
		Uint32 _newPathTimer;

		int _map[WORLD_SIZE][WORLD_SIZE];
		int _oldMapPosX;
		int _oldMapPosZ;

		// Private functions
		bool _checkLine(int levelmap[WORLD_SIZE][WORLD_SIZE], glm::vec3 A, glm::vec3 B);
		void _alien(float delta);
		void _robot(float delta);
		void _alienBoss(float delta);
		void _alienSpawner(float delta);
		void _robotSpawner(float delta);
		void _mapUpdateEnemy();
	};
};
