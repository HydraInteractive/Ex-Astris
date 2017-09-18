/**
 * Everything related to the view, as an interface.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <cstdint>

#include <glm/glm.hpp>
#include <hydra/renderer/renderer.hpp>

namespace Hydra::Renderer { class HYDRA_API IUIRenderer; }

namespace Hydra::View {
	class HYDRA_API IView : public Hydra::Renderer::IRenderTarget {
	public:
		virtual ~IView() = 0;

		/// Run the event loop stuff
		virtual void update(Hydra::Renderer::IUIRenderer* uiRenderer = nullptr) = 0;

		virtual void show() = 0;
		virtual void hide() = 0;
		virtual void quit() = 0;

		virtual void* getHandler() = 0;
		virtual void finalize() = 0;
		inline void bind(size_t position) final {}
		inline size_t getSamples() final { return 1; }

		/// Did it get a close event, and hid itself
		virtual bool isClosed() = 0;
		virtual bool didChangeSize() = 0;
	};
	inline IView::~IView() {}
}
