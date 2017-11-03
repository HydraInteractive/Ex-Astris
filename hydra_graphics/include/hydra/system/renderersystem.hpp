#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API RendererSystem final : public Hydra::World::ISystem{
	public:
		RendererSystem();
		~RendererSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "RendererSystem"; }
		void registerUI() final;
	};
}
