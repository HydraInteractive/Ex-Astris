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
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/component/playercomponent.hpp>
#include <math.h>
#include <SDL2\SDL.h>

using namespace Hydra::World;
namespace Hydra::Component {

	enum class EnemyTypes {
		Alien = 0,
		Robot = 1,
		AlienBoss = 2,
	};

	enum PathState{
		SEARCHING = 0,
		FOUND_GOAL = 1,
		ATTACKING = 2,
	};

	class HYDRA_API EnemyComponent final : public IComponent{
	public:
		EnemyComponent(IEntity* entity);
		EnemyComponent(IEntity* entity, EnemyTypes enemyID);
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
		int _debugState;
		unsigned int lastTime, currentTime;
		float angle;
		glm::quat rotation;
		int map[WORLD_SIZE][WORLD_SIZE];
		float _velocityX;
		float _velocityY;
		float _velocityZ;
		glm::vec3 _targetPos;
		glm::vec3 _position;
		glm::vec3 _startPosition;
		glm::quat _rotation;
		bool _isAtGoal;
		bool _falling;
		bool _patrolPointReached;
		EnemyTypes _enemyID = EnemyTypes::Alien;
	};
};
