#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <hydra/renderer/renderer.hpp>

namespace Hydra::Renderer { class IUIRenderer; }

namespace Hydra::View {
	class IView : public Hydra::Renderer::IRenderTarget {
	public:
		virtual ~IView() = 0;

		/// Run the event loop stuff
		virtual void update(Hydra::Renderer::IUIRenderer* uiRenderer = nullptr) = 0;

		virtual void show() = 0;
		virtual void hide() = 0;
		virtual void quit() = 0;

		virtual void* getHandler() = 0;
		virtual void finalize() = 0;

		/// Did it get a close event, and hid itself
		virtual bool isClosed() = 0;
		virtual bool didChangeSize() = 0;
	};
	inline IView::~IView() {}
}
