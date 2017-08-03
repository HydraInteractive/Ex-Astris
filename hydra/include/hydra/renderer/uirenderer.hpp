#pragma once

#include <hydra/view/view.hpp>
#include <memory>

// TODO: Abstact away?
union SDL_Event;

namespace Hydra::Renderer {
	class IUIRenderer {
	public:
		virtual ~IUIRenderer() = 0;

		virtual void handleEvent(SDL_Event& event) = 0;

		virtual void newFrame() = 0;

		virtual void render() = 0; // TODO: Move to IRenderer(?)

		virtual bool usingKeyboard() = 0;
	};
	inline IUIRenderer::~IUIRenderer() {}

	namespace UIRenderer {
		std::unique_ptr<IUIRenderer> create(Hydra::View::IView& view);
	};
};
