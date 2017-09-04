#include <hydra/io/gltextureloader.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/engine.hpp>

#include <exception>

using namespace Hydra;
using namespace Hydra::IO;

class HYDRA_API TextureLoaderImpl final : public ITextureLoader {
public:
	TextureLoaderImpl() : _errorTexture(_loadErrorTexture()) {}

	~TextureLoaderImpl() final {
		_storage.clear();
	}

	std::shared_ptr<ITexture> getTexture(const std::string& file) final {
		if (file.empty())
			return _errorTexture;
		std::shared_ptr<ITexture> texture = _storage[file];
		if (!texture) {
			try {
				IEngine::getInstance()->log(LogLevel::verbose, "Loading texture: %s", file.c_str());
				texture = _storage[file] = Hydra::Renderer::GLTexture::createFromFile(file);
			}
			catch (const std::exception& e) {
				IEngine::getInstance()->log(LogLevel::error, "FAILED TO LOAD TEXTURE: %s", e.what());
				return _errorTexture;
			}
		}
		return texture;
	}

	std::shared_ptr<ITexture> getErrorTexture() final {
		return _errorTexture;
	}

private:
	std::map<std::string, std::shared_ptr<ITexture>> _storage;
	std::shared_ptr<ITexture> _errorTexture;


	std::shared_ptr<ITexture> _loadErrorTexture() {
		return Hydra::Renderer::GLTexture::createFromFile("assets/textures/error.png");
	}
};

std::unique_ptr<ITextureLoader> GLTextureLoader::create() {
	return std::unique_ptr<ITextureLoader>(new TextureLoaderImpl());
}