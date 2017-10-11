#pragma once

#include <hydra/world/world.hpp>

class AISystem final : public Hydra::World::ISystem {
public:
	~AISystem() final;

	void tick(float delta) final;

	inline const std::string type() const final { return "AISystem"; }
	void registerUI() final;
};
