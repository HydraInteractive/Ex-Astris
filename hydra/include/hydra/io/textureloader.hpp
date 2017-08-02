#pragma once

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

namespace Hydra::IO {
	class TextureLoader final {
	public:
		TextureLoader();
		virtual ~TextureLoader();

		std::shared_ptr<ITexture> getTexture(const std::string& file);
		std::shared_ptr<ITexture> getErrorTexture();

	private:
		std::map<std::string, std::shared_ptr<Renderer::ITexture>> _storage;
		std::shared_ptr<ITexture> _errorTexture;

		std::shared_ptr<ITexture> _loadErrorTexture();
	};
};
