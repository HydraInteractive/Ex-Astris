/**
* Weapon stuff
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
	struct HYDRA_PHYSICS_API WeaponComponent final : public IComponent<WeaponComponent, ComponentBits::Weapon> {
		BulletType bulletType = BULLETTYPE_NORMAL;
		float fireRateTimer = 0;
		float fireRateRPM = 600;
		float bulletSize = 0.5f;
		float bulletSpread = 0.0f;
		int bulletsPerShot = 14;
		float recoil = 0.7;

		float _dyaw, _dpitch = 0;
		bool _isReloading = false;

		/*unsigned short*/ int maxammo = 500;
		/*unsigned short*/ int currammo = 500;
		/*unsigned short*/ int maxmagammo = 10;
		/*unsigned short*/ int currmagammo = 10;
		/*unsigned short*/ int ammoPerShot = 1;
		float reloadTime = 0;
		float maxReloadTime = 2;

		~WeaponComponent() final;

		bool shoot(glm::vec3 position, glm::vec3 direction, glm::quat bulletOrientation, float velocity, Hydra::System::BulletPhysicsSystem::CollisionTypes collisionType, int damage);
		bool reload(float delta);
		void resetReload();

		inline const std::string type() const final { return "WeaponComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
