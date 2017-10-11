#pragma once

#include <hydra/world/world.hpp>

class AbilitySystem final : public Hydra::World::ISystem {
public:
	~AbilitySystem() final;

	void tick(float delta) final;

	inline const std::string type() const final { return "AbilitySystem"; }
	void registerUI() final;
};
