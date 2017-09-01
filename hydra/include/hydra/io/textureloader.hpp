#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

//TODO: Use PBO! http://www.songho.ca/opengl/gl_pbo.html#unpack

namespace Hydra::IO {
	class HYDRA_API ITextureLoader {
	public:
		virtual ~ITextureLoader() = 0;

		virtual std::shared_ptr<ITexture> getTexture(const std::string& file) = 0;
		virtual std::shared_ptr<ITexture> getErrorTexture() = 0;
	};
	inline ITextureLoader::~ITextureLoader() {}
};
