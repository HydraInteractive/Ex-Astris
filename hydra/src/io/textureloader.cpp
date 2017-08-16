#include <hydra/io/textureloader.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/engine.hpp>

#include <exception>

using namespace Hydra;
using namespace Hydra::IO;

TextureLoader::TextureLoader() : _errorTexture(_loadErrorTexture()) {}

TextureLoader::~TextureLoader() {
	_storage.clear();
}

std::shared_ptr<ITexture> TextureLoader::getTexture(const std::string& file) {
	if (file.empty())
		return _errorTexture;
	std::shared_ptr<ITexture> texture = _storage[file];
	if (!texture) {
		try {
			IEngine::getInstance()->log(LogLevel::verbose, "Loading texture: %s", file.c_str());
			texture = _storage[file] = Hydra::Renderer::GLTexture::createFromFile(file);
		} catch (const std::exception& e) {
			IEngine::getInstance()->log(LogLevel::error, "FAILED TO LOAD TEXTURE: %s", e.what());
			return _errorTexture;
		}
	}

	return texture;
}

std::shared_ptr<ITexture> TextureLoader::getErrorTexture() {
	return _errorTexture;
}

std::shared_ptr<ITexture> TextureLoader::_loadErrorTexture() {
	return Hydra::Renderer::GLTexture::createFromFile("assets/textures/error.png");
}
