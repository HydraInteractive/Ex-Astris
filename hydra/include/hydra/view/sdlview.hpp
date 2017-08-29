#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/view/view.hpp>

#include <memory>

namespace Hydra::View {
	struct HYDRA_API SDLView final {
		SDLView() = delete;

		static std::unique_ptr<IView> create();
	};
}
