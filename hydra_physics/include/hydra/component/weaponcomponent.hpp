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
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/system/bulletphysicssystem.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API WeaponComponent final : public IComponent<WeaponComponent, ComponentBits::Weapon> {
		BulletComponent::BulletType bulletType = BulletComponent::BulletType::normal;
		float fireRateTimer = 0.0f;
		float fireRateRPM = 600.0f;
		float bulletSize = 0.5f;
		float bulletSpread = 0.0f;
		float damage = 5.0f;
		int bulletsPerShot = 14;
		float recoil = 0.7f;
		float glowIntensity = 0.0f;
		float color[4] = { 0.0f };
		float _dyaw, _dpitch = 0.0f;
		bool _isReloading = false;
		bool glow = false;

		/*unsigned short*/ int maxammo = 100000000;
		/*unsigned short*/ int currammo = 100000000;
		/*unsigned short*/ int maxmagammo = 25;
		/*unsigned short*/ int currmagammo = 25;
		/*unsigned short*/ int ammoPerShot = 1;
		float reloadTime = 0.0f;
		float maxReloadTime = 2.0f;

		~WeaponComponent() final;

		bool shoot(glm::vec3 position, glm::vec3 direction, glm::quat bulletOrientation, float velocity, Hydra::System::BulletPhysicsSystem::CollisionTypes collisionType);
		bool reload(float delta);
		void resetReload();

		inline const std::string type() const final { return "WeaponComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};
