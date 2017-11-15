#pragma once

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API AnimationSystem final : public Hydra::World::ISystem{
	public:
		AnimationSystem();
		~AnimationSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "AnimationSystem"; }
		void registerUI() final;
	};
}
