#include <hydra/io/textureloader.hpp>

#include <hydra/renderer/glrenderer.hpp>

#include <exception>
#include <iostream>

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
			std::cout << "Loading texture: " << file << std::endl;
			texture = _storage[file] = Hydra::Renderer::GLTexture::createFromFile(file);
		} catch (const std::exception& e) {
			std::cerr << "FAILED TO LOAD TEXTURE: " << e.what() << std::endl;
			return _errorTexture;
		} catch (const char* msg) {
			std::cerr << "FAILED TO LOAD TEXTURE: " << msg << std::endl;
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
