#pragma once

#include <hydra/view/view.hpp>

namespace Hydra::View {
	struct SDLView final {
		SDLView() = delete;

		static IView* create();
	};
}
