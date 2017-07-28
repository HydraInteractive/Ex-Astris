#pragma once

#include <glm/glm.hpp>

namespace Hydra::View {
	class IRenderTarget;
	
	class IView {
	public:
		virtual ~IView() = 0;

		/// Run the event loop stuff
		virtual void update() = 0;

		virtual void show() = 0;
		virtual void hide() = 0;

		virtual IRenderTarget& getRenderTarget() = 0; 
		virtual glm::ivec2 getSize() = 0;

		/// Did it get a close event, and hid itself
		virtual bool isClosed() = 0;
		virtual bool didChangeSize() = 0;
	};
	
	inline IView::~IView() {}

	class IRenderTarget {
	public:
		virtual ~IRenderTarget() = 0;

		virtual void bind() = 0;
		
		virtual void clear() = 0;		
		
		/// Push the rendered pixels to the screen
		virtual void push() = 0;
	};

	inline IRenderTarget::~IRenderTarget() {}
}
