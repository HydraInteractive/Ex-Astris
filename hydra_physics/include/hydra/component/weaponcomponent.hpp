#pragma once
/**
* Player stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API WeaponComponent final : public IComponent{
	public:
		WeaponComponent(IEntity* entity);
		~WeaponComponent() final;

		void tick(TickAction action, float delta) final;
		inline TickAction wantTick() const final { return TickAction::physics; }

		void shoot(glm::vec3 position, glm::vec3 direction, glm::quat bulletOrientation, float velocity);
		std::shared_ptr<Hydra::World::IEntity> getBullets();

		inline const std::string type() const final { return "WeaponComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	private:
		unsigned int _fireRateTimer = 0;
		int _fireRateRPM = 60;
		float _bulletSize = 0.1;
		int _bulletsPerShot = 200;
		glm::vec3 _debug;
	};
};
