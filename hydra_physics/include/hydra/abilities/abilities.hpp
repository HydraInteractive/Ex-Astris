#pragma once
#include <hydra/world/world.hpp>
#include <hydra/engine.hpp>
#include <hydra/ext/openmp.hpp>

using world = Hydra::World::World;

struct BaseAbility {
	virtual void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity) = 0;
	virtual void tick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity) {};
	virtual void doneTick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity) {};
	int cooldown = 1;
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

struct BulletSprayAbillity: public BaseAbility {
	BulletSprayAbillity() { tickFreq = 0.1f; }
	void useAbility(const std::shared_ptr<Hydra::World::Entity>& playerEntity);
	void tick(float delta, const std::shared_ptr<Hydra::World::Entity>& playerEntity);
};
