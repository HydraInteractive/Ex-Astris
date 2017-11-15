#include <hydra/io/textfactory.hpp>
#include <hydra/io/gltextfactory.hpp>
#include <glm/glm.hpp>
#include <hydra/engine.hpp>
using namespace Hydra;
using namespace IO;

class HYDRA_GRAPHICS_API TextFactoryImpl final : public ITextFactory{
public:
	TextFactoryImpl(const std::string& fontFile) {
		_fontMap = IEngine::getInstance()->getState()->getTextureLoader()->getTexture(fontFile);
		float w = 1.0 / 16.0f;
		float h = w;

		for (int i = 0; i < 256; i++) {
			auto& info = _charInfos[i];
			info.pos = glm::vec2{w * (i % 16), h * (i / 16)};
			info.size = glm::vec2{w, h};
			info.xAdvanceAmount = w;
		}

	}
	~TextFactoryImpl() final {
		
	}

	std::shared_ptr<ITexture> getTexture() final { return _fontMap; }
	inline const CharInfo& getChar(char ch) final { return _charInfos[(int)ch]; }

private:
	std::shared_ptr<ITexture> _fontMap;
	CharInfo _charInfos[256];
};

std::unique_ptr<ITextFactory> GLTextFactory::create(const std::string& fontFile) {
	return std::unique_ptr<ITextFactory>(new TextFactoryImpl(fontFile));
}