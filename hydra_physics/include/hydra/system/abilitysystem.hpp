#pragma once

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>

namespace Hydra::System {
	class HYDRA_PHYSICS_API AbilitySystem final : public Hydra::World::ISystem {
	public:
		AbilitySystem();
		~AbilitySystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "AbilitySystem"; }
		void registerUI() final;

	private:
		void _spawnParticleEmitterAt(const glm::vec3& pos, const glm::vec3& normal);
	};
}
