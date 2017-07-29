#pragma once

#include <hydra/renderer/renderer.hpp>
#include <hydra/view/view.hpp>

#include <memory>

namespace Hydra::Renderer {
	struct GLRenderer final {
		GLRenderer() = delete;

		static std::unique_ptr<IRenderer> create(Hydra::View::IView& view);
	};
}
