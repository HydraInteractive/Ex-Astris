/**
 * An implementation for the ITextureLoader, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <hydra/io/textureloader.hpp>

namespace Hydra::IO {
	namespace GLTextureLoader {
		HYDRA_API std::unique_ptr<ITextureLoader> create();
	};
};
