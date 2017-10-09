#pragma once

#include <hydra/world/world.hpp>

class DeadSystem final : public Hydra::World::ISystem {
public:
	~DeadSystem() final;

	void tick(float delta) final;

	inline const std::string type() const final { return "DeadSystem"; }
	void registerUI() final;
};
