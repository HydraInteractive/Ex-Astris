#pragma once
#include <hydra/ext/api.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>
#include <hydra/engine.hpp>
#include <hydra/io/textfactory.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

// TODO: Implement LOD

namespace Hydra::Component {
	struct HYDRA_GRAPHICS_API TextComponent final : public IComponent<TextComponent, ComponentBits::Text>{

	std::vector<CharRenderInfo> renderingData;
	std::string text;
	size_t maxLength = 24;
	bool isStatic = true;

	void rebuild();
	void setText(const std::string& inText);

	~TextComponent() final;

	inline const std::string type() const final { return "TextComponent"; }

	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};
