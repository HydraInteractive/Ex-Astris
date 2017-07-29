#pragma once

#include <hydra/view/view.hpp>

#include <memory>

namespace Hydra::View {
	struct SDLView final {
		SDLView() = delete;

		static std::unique_ptr<IView> create();
	};
}
