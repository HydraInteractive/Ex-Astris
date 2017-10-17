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

	enum PathState {
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

	struct HYDRA_PHYSICS_API EnemyComponent final : public IComponent<EnemyComponent, ComponentBits::Enemy> {
		// TODO: !!!!!REMOVE!!!!!
		void init(EnemyTypes enemyID, glm::vec3 pos, int hp, int dmg, float range, glm::vec3 scale);
		~EnemyComponent() final;

		// TODO: !!!!!REMOVE!!!!!
		void tick(float delta);

		inline const std::string type() const final { return "EnemyComponent"; }

		float getRadius();
		std::shared_ptr<Hydra::World::Entity> getPlayerComponent();
		int getWall(int x, int y);

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		PathState _pathState = IDLE;
		PathFinding* _pathFinding = new PathFinding();
		BossPhase _bossPhase = CLAWING;
		std::vector<std::shared_ptr<Hydra::World::Entity>> _spawnGroup;
		int _debugState;
		float _angle;
		float _velocityX = 0;
		float _velocityY = 0;
		float _velocityZ = 0;
		int _spawnAmount = 0;
		int _health = 1;
		int _damage = 0;
		float _range = 1;
		float _originalRange = 1;
		glm::vec3 _mapOffset = glm::vec3{0, 0, 0};
		glm::vec3 _targetPos;
		glm::vec3 _position = glm::vec3{0, 0, 0};
		glm::vec3 _startPosition = glm::vec3{0, 0, 0};
		glm::quat _rotation = glm::quat();
		glm::vec3 _scale = glm::vec3{1, 1, 1};
		bool _isAtGoal;
		bool _falling = false;
		bool _patrolPointReached = false;
		bool _playerSeen;
		bool _stunned = false;
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
	};
};
