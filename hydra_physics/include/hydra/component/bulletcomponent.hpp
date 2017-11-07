/**
* Bullet stuff
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

using namespace Hydra::World;

enum BulletType {
	BULLETTYPE_NORMAL,
	BULLETTYPE_MAGNETIC,
	BULLETTYPE_HOMING
};


namespace Hydra::Component {
	struct HYDRA_PHYSICS_API BulletComponent final : public IComponent<BulletComponent, ComponentBits::Bullet> {
		 BulletType bulletType = BULLETTYPE_HOMING;
		 glm::vec3 direction;
		 float velocity;
		 float deleteTimer = 10;
		 float damage = 30.0f;

		~BulletComponent() final;

		inline const std::string type() const final { return "BulletComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
