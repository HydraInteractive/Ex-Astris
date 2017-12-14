#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API SpawnerSystem final : public Hydra::World::ISystem{
	public:
		std::vector<Entity*> didJustSpawn;

		SpawnerSystem();
		~SpawnerSystem() final;

		void tick(float delta) final;
		inline const std::string type() const final { return "SpawnerSystem"; }
		void registerUI() final;
	};

}