/**
* EnemyComponent/AI.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#include <hydra/ext/api.hpp>
#include <glm\glm.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>

using namespace Hydra::World;
namespace Hydra::Component {
	enum class EnemyTypes {
		Alien = 0,
		Robot = 1,
		AlienBoss = 2,

	};
	class HYDRA_API EnemyComponent final : public IComponent{
	public:
		EnemyComponent(IEntity* entity);
		EnemyComponent(IEntity* entity, EnemyTypes enemyID);
		~EnemyComponent() final;

		void tick(TickAction action) final;
		// If you want to add more than one TickAction, combine them with '|' (The bitwise or operator) 
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "EnemyComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	private:
		float _velocityX;
		float _velocityY;
		float _velocityZ;
		glm::vec3 _position;
		glm::vec3 _startPosition;
		bool _falling;
		bool _patrolPointReached;
		EnemyTypes _enemyID = EnemyTypes::Alien;

		int _a = 0;
		bool _b = false;
		float _c = 0.0f;
	};
};