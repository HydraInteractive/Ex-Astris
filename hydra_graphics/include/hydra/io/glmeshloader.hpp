/**
 * An implementation for the IMeshLoader, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <hydra/io/meshloader.hpp>

namespace Hydra::IO {
	namespace GLMeshLoader {
		HYDRA_GRAPHICS_API std::unique_ptr<IMeshLoader> create(IRenderer* renderer);
	};
};
