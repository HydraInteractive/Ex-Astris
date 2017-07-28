#pragma once

#include <hydra/view/view.hpp>

namespace Hydra::View {
	struct SDLView final {
		static IView* create();
	};
}
