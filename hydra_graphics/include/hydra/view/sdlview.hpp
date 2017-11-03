/**
 * An implementation for the IView, using SDL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/view/view.hpp>

#include <memory>

namespace Hydra::View {
	namespace SDLView {
		HYDRA_GRAPHICS_API std::unique_ptr<IView> create();
	};
}
