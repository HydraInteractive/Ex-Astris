#pragma once
#include <hydra/ext/api.hpp>

#include <memory>

#include <hydra/world/world.hpp>
#include <glm/glm.hpp>

using namespace Hydra::World;

// TODO: Implement LOD

namespace Hydra::Component {
	struct HYDRA_GRAPHICS_API TextComponent final : public IComponent<TextComponent, ComponentBits::Text>{

	struct CharRenderInfo {
		// charRect.xy = [startX, startY] in texture
		// charRect.zw = [width, height] of char
		// charPos     = [xPos, yPos, zPos]
		glm::vec4 charRect;
		glm::vec3 charPos;
	};

	std::vector<CharRenderInfo> renderingData;

	~TextComponent() final;

	inline const std::string type() const final { return "TextComponent"; }

	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};
