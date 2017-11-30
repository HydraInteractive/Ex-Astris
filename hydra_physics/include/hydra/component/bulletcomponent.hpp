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
#include <hydra/world/world.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API BulletComponent final : public IComponent<BulletComponent, ComponentBits::Bullet> {
		enum class BulletType : size_t {
			normal = 0,
			magnetic,
			homing
		};

		BulletType bulletType = BulletType::homing;
		glm::vec3 direction = glm::vec3(0.0f,0.0f,0.0f);
		float velocity = 0.0f;
		float deleteTimer = 10.0f;
		float damage = 30.0f;

		~BulletComponent() final;

		inline const std::string type() const final { return "BulletComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
