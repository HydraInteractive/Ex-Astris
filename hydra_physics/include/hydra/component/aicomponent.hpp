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

	struct HYDRA_PHYSICS_API AIComponent final : public IComponent<AIComponent, ComponentBits::AI> {
		std::shared_ptr<Behaviour> behaviour;

		glm::vec3 _scale = glm::vec3{ 1, 1, 1 };

		float range = 1;
		float originalRange = 1;
		glm::vec3 mapOffset = glm::vec3(-30.0f, 0, -30.0f);
		glm::vec3 targetPos = glm::vec3{ 0, 0, 0 };
		glm::quat rotation = glm::quat();
		float angle = 1;
		int oldMapPosX = 0;
		int oldMapPosZ = 0;
		int debugState;
		int damage = 0;
		int spawnAmount = 0;
		int map[WORLD_SIZE][WORLD_SIZE];
		bool isAtGoal = false;
		bool patrolPointReached = false;
		bool playerSeen = false;
		bool stunned = false;
		std::random_device rd;
		float idleTimer = 0;
		float stunTimer = 0;
		float attackTimer = 0;
		float newPathTimer = 0;
		float spawnTimer = 0;
		float phaseTimer = 0;

		~AIComponent() final;


		inline const std::string type() const final { return "AIComponent"; }

		std::shared_ptr<Hydra::World::Entity> getPlayerEntity();

		float getRadius(){ return _scale.x; }

		bool checkLOS(int levelmap[WORLD_SIZE][WORLD_SIZE], glm::vec3 A, glm::vec3 B);

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		int getWall(int x, int y) { return map[x][y]; }
	};
};
