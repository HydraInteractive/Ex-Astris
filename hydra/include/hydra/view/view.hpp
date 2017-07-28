#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace Hydra::View {
	class IRenderTarget {
	public:
		virtual ~IRenderTarget() = 0;

		virtual uint32_t getID() = 0;

		virtual void finalize() = 0;
	};

	inline IRenderTarget::~IRenderTarget() {}

	class IView : public IRenderTarget {
	public:
		virtual ~IView() = 0;

		/// Run the event loop stuff
		virtual void update() = 0;

		virtual void show() = 0;
		virtual void hide() = 0;

		virtual glm::ivec2 getSize() = 0;
		virtual void* getHandler() = 0;

		/// Did it get a close event, and hid itself
		virtual bool isClosed() = 0;
		virtual bool didChangeSize() = 0;
	};

	inline IView::~IView() {}
}
