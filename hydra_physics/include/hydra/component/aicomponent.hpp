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
		PathState _pathState = IDLE;
		PathFinding* _pathFinding = new PathFinding();
		BossPhase _bossPhase = CLAWING;
		std::vector<std::shared_ptr<Hydra::World::Entity>> _spawnGroup;
		EnemyTypes _enemyID = EnemyTypes::Alien;
		glm::vec3 _velocity = glm::vec3{ 0, 0, 0 };
		glm::vec3 _mapOffset = glm::vec3(-30.0f, 0, -30.0f);
		glm::vec3 _targetPos = glm::vec3{ 0, 0, 0 };
		glm::vec3 _position = glm::vec3{ 0, 0, 0 };
		glm::vec3 _startPosition = glm::vec3{ 0, 0, 0 };
		glm::vec3 _scale = glm::vec3{ 1, 1, 1 };
		glm::quat _rotation = glm::quat();
		float _angle = 1;
		float _range = 1;
		float _originalRange = 1;
		int _debugState;
		int _spawnAmount;
		int _damage = 0;
		int _map[WORLD_SIZE][WORLD_SIZE];
		int _oldMapPosX = 0;
		int _oldMapPosZ = 0;
		bool _isAtGoal = false;
		bool _falling = false;
		bool _patrolPointReached = false;
		bool _playerSeen = false;
		bool _stunned = false;
		std::random_device rd;
		float _timer = 0;
		float _spawnTimer = 0;
		float _stunTimer = 0;
		float _attackTimer = 0;
		float _newPathTimer = 0;

		~EnemyComponent() final;


		inline const std::string type() const final { return "EnemyComponent"; }

		std::shared_ptr<Hydra::World::Entity> EnemyComponent::getPlayerComponent();

		float getRadius(){ return _scale.x; }

		int getWall(int x, int y) { return _map[x][y]; }

		bool _checkLOS(int levelmap[WORLD_SIZE][WORLD_SIZE], glm::vec3 A, glm::vec3 B);

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	};
};
