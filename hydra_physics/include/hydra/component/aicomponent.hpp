/**
* AIComponent/AI.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once
#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/pathing/behaviour.hpp>
#include <math.h>
#include <SDL2/SDL.h>
#include <random>


using namespace Hydra::World;
namespace Hydra::Component {

	enum BossPhase {
		CLAWING = 0,
		SPITTING = 1,
		SPAWNING = 2,
		CHILLING = 3
	};

	struct HYDRA_PHYSICS_API AIComponent final : public IComponent<AIComponent, ComponentBits::AI> {
		BossPhase bossPhase = CLAWING;
		std::shared_ptr<Behaviour> behaviour;
		std::vector<std::shared_ptr<Hydra::World::Entity>> spawnGroup;

		glm::vec3 mapOffset = glm::vec3(-30.0f, 0, -30.0f);
		glm::vec3 _targetPos = glm::vec3{ 0, 0, 0 };
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

		~AIComponent() final;


		inline const std::string type() const final { return "AIComponent"; }

		std::shared_ptr<Hydra::World::Entity> AIComponent::getPlayerEntity();

		float getRadius(){ return _scale.x; }

		int getWall(int x, int y) { return _map[x][y]; }

		bool _checkLOS(int levelmap[WORLD_SIZE][WORLD_SIZE], glm::vec3 A, glm::vec3 B);

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	};
};
