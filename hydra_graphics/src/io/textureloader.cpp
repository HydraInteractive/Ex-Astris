// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the ITextureLoader, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/io/gltextureloader.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/engine.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <exception>

using namespace Hydra;
using namespace IO;
using namespace Renderer;

class HYDRA_GRAPHICS_API TextureLoaderImpl final : public ITextureLoader {
public:
	TextureLoaderImpl() : _errorTexture(_loadErrorTexture()) {
		if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
			printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		}
	}

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
				texture = _storage[file] = GLTexture::createFromFile(file);
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
		return GLTexture::createFromFile("assets/textures/error.png");
	}
};

std::unique_ptr<ITextureLoader> GLTextureLoader::create() {
	return std::unique_ptr<ITextureLoader>(new TextureLoaderImpl());
}
