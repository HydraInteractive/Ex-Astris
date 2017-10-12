#pragma once

#include <hydra/ext/api.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_GRAPHICS_API DrawObjectComponent final : public IComponent<DrawObjectComponent, ComponentBits::DrawObject> {
		Hydra::Renderer::DrawObject* drawObject;

		DrawObjectComponent();
		~DrawObjectComponent() final;

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
		void registerUI() final;

		const std::string type() const final { return "DrawObjectComponent"; }
	};
}
