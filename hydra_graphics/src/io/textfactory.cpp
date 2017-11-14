#include <hydra/io/textfactory.hpp>
#include <hydra/io/gltextfactory.hpp>
#include <glm/glm.hpp>
using namespace Hydra;
using namespace IO;

class HYDRA_GRAPHICS_API TextFactoryImpl final : public ITextFactory{
public:
	TextFactoryImpl() {
		float w = 1.0 / 16.0f;
		float h = w;

		for (int i = 0; i < 0x100; i++) {
			
		}

	}
	~TextFactoryImpl() final {
		
	}

	struct CharInfo {
		glm::vec2 pos;
		glm::vec2 size;

		// glm::vec2 offset;
		float xAdvanceAmount;
	};
private:
	const int _ptSize = 30;
	std::shared_ptr<ITexture> _fontMap;
	CharInfo _charInfos[256];
};

std::unique_ptr<ITextFactory> GLTextFactory::create() {
	return std::unique_ptr<ITextFactory>(new TextFactoryImpl());
}