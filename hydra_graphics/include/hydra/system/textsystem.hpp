#pragma once

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>

namespace Hydra::System {
	class HYDRA_GRAPHICS_API TextSystem final : public Hydra::World::ISystem{
	public:

		TextSystem();
		~TextSystem() final;

		void tick(float delta) final;

		inline const std::string type() const final { return "TextSystem"; }
		void registerUI() final;
	private:

	};
}
