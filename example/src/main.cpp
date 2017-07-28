#include <hydra/engine.hpp>

#include <hydra/view/sdlview.hpp>
#include <hydra/renderer/glrenderer.hpp>

class Engine final : public Hydra::Engine {
public:
	Engine() {
		_view = Hydra::View::SDLView::create();
		_renderer = Hydra::Renderer::GLRenderer::create(_view);
	}

	virtual ~Engine() final {
		delete _renderer;
		delete _view;
	}

	void run() final {
		while (!_view->isClosed()) {
			_view->update();

			_renderer->bind(_view);
			_renderer->clear(glm::vec4{0, 0.2, 0.2, 1});

			_view->finalize();
		}
	}

private:
	Hydra::View::IView* _view;
	Hydra::Renderer::IRenderer* _renderer;
};

int main(int argc, const char** argv) {
	Engine().run();
	return 0;
}
