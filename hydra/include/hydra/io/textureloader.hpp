/**
 * A texture loader that caches the loaded textures.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
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
