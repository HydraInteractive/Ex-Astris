#pragma once
/**
* Mine stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/


#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API MineComponent final : public IComponent{
	public:
		MineComponent(IEntity* entity);
		MineComponent(IEntity* entity, glm::vec3 position, glm::vec3 direction);
		~MineComponent() final;

		void tick(TickAction action, float delta) final;
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "GrenadeComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	private:
		glm::vec3 _position;
		glm::vec3 _direction;
		float _velocity;
		float _fallingVelocity;
		Uint32 _tickSpawned;
		Uint32 _timeUntilDelete;
	};
};