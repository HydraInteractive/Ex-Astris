#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

//TODO: Use PBO! http://www.songho.ca/opengl/gl_pbo.html#unpack

namespace Hydra::IO {
	class HYDRA_API TextureLoader final {
	public:
		TextureLoader();
		virtual ~TextureLoader();

		std::shared_ptr<ITexture> getTexture(const std::string& file);
		std::shared_ptr<ITexture> getErrorTexture();

	private:
		std::map<std::string, std::shared_ptr<ITexture>> _storage;
		std::shared_ptr<ITexture> _errorTexture;

		std::shared_ptr<ITexture> _loadErrorTexture();
	};
};
