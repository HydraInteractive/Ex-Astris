#pragma once
#include <hydra/world/world.hpp>
#include <hydra/engine.hpp>
#include <hydra/ext/openmp.hpp>

#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/soundfxcomponent.hpp>
#include <hydra/component/rigidbodycomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/abilities/grenadecomponent.hpp>

using world = Hydra::World::World;

struct BaseAbility {
	virtual void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity) = 0;
	virtual void tick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity) {};
	int cooldown = 0;
	float activeTimer = 0;
	float tickFreq = 0;
	float timeSinceLastTick = 0;
};

struct GrenadeAbility : public BaseAbility {
	GrenadeAbility() {}
	void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity);
};

struct MineAbility : public BaseAbility {
	MineAbility() {}
	void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity);
};

struct forcePushAbility : public BaseAbility {
	forcePushAbility() {}
	void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity);
};

struct BulletSprayAbillity: public BaseAbility {
	BulletSprayAbillity() { tickFreq = 0.1f; }
	void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity);
	void tick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity);
};

struct DashAbility : public BaseAbility {
	DashAbility() {}
	void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity);
};