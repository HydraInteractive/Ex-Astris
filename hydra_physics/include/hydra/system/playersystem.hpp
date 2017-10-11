#pragma once

#include <hydra/world/world.hpp>

class PlayerSystem final : public Hydra::World::ISystem {
public:
	~PlayerSystem() final;

	void tick(float delta) final;

	inline const std::string type() const final { return "PlayerSystem"; }
	void registerUI() final;
};
