#include <hydra/engine.hpp>

#include <cstdio>

#include <hydra/view/view.hpp>
#include <hydra/view/sdlview.hpp>

class HydraEngine final : public Hydra::Engine {
public:
	HydraEngine() {
		view = Hydra::View::SDLView::create();
	}

	virtual ~HydraEngine() final{
		delete view;
	}
	
	void run() final {
		while (!view->isClosed()) {
			view->update();
			auto& rt = view->getRenderTarget();
			rt.bind();
			rt.clear();
			rt.push();
		}
	}

private:
	Hydra::View::IView* view;
};

Hydra::Engine& Hydra::Engine::getInstance() {
	static HydraEngine instance;
	return static_cast<Hydra::Engine&>(instance);
}
