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
#include <hydra/abilities/grenadecomponent.hpp>
#include <hydra/abilities/minecomponent.hpp>

using world = Hydra::World::World;

struct BaseAbility {
	virtual void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity) = 0;
	virtual void tick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity) {};
	virtual void doneTick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity) { printf("AFTER LAST TICK DONE\n"); };
	int cooldown = 0;
	float activeTimer = 0;
	float tickFreq = 0;
	float timeSinceLastTick = 0;
	bool afterLastTick = false;
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