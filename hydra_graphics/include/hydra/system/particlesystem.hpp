#pragma once

#include <hydra/world/world.hpp>

class ParticleSystem final : public Hydra::World::ISystem {
public:
	~ParticleSystem() final;

	void tick(float delta) final;

	inline const std::string type() const final { return "ParticleSystem"; }
	void registerUI() final;
};
