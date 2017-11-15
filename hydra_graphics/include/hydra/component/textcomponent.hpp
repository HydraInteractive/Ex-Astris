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
	int maxLength = 10;

	void rebuild() {
		renderingData.resize(text.size());
		auto factory = IEngine::getInstance()->getState()->getTextFactory();
		glm::vec3 pos{ 0 };
		for (unsigned int i = 0; i < text.size(); i++) {
			const Hydra::IO::ITextFactory::CharInfo& info = factory->getChar(text[i]);
			pos.x -= info.xAdvanceAmount * 1.2;
		}
		pos /= -2;

		for (unsigned int i = 0; i < text.size(); i++) {
			const Hydra::IO::ITextFactory::CharInfo& info = factory->getChar(text[i]);
			CharRenderInfo& rInfo = renderingData[i];
			rInfo.charRect = glm::vec4{info.pos, info.size};
			rInfo.charPos = pos;
			pos.x -= info.xAdvanceAmount * 1.2;
		}
	}

	~TextComponent() final;

	inline const std::string type() const final { return "TextComponent"; }

	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};
