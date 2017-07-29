#pragma once

#include <hydra/renderer/renderer.hpp>
#include <hydra/view/view.hpp>

namespace Hydra::Renderer {
	struct GLRenderer final {
		GLRenderer() = delete;

		static IRenderer* create(Hydra::View::IView* view);
	};
}
