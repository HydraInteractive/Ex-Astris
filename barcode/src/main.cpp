#include <hydra/engine.hpp>

#include <SDL2/SDL.h>

#include <memory>

#include <hydra/view/sdlview.hpp>
#include <hydra/renderer/glrenderer.hpp>

#include <hydra/component/componentmanager.hpp>
#include <hydra/component/componentmanager_graphics.hpp>
#include <hydra/component/componentmanager_network.hpp>
#include <hydra/component/componentmanager_physics.hpp>
#include <hydra/component/componentmanager_sound.hpp>

#include <barcode/menustate.hpp>
#include <barcode/gamestate.hpp>
#include <barcode/editorstate.hpp>

#include <cstdio>
#include <imgui/imgui.h>

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

static inline void reportMemoryLeaks() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}
#else
static inline void reportMemoryLeaks() {}
#endif

namespace Barcode {
	using namespace Hydra;
	class Engine final : public IEngine {
	public:
		Engine() {
			IEngine::getInstance() = this;

			_view = View::SDLView::create();
			_renderer = Renderer::GLRenderer::create(*_view);
			_uiRenderer = Renderer::UIRenderer::create(*_view);

			_setupComponents();
		}

		~Engine() final { setState_(nullptr); }

		void run() final {
			_state = std::move(_newState);
			_uiRenderer->reset();
			_state->load();
			_quit = false;

			while (!_quit && _state && !_view->isClosed()) {
				{ // Remove old dead objects
					_state->getWorld()->tick(TickAction::checkDead);
					_renderer->cleanup();
				}

				_state->runFrame();
				_uiRenderer->render();
				_view->finalize();

				if (_newState) {
					_state = std::move(_newState);
					_uiRenderer->reset();
					_state->load();
				}
			}
		}

		void quit() final { _quit = true; }

		void onMainMenu() final {
			if (ImGui::BeginMenu("States")) {
				if (ImGui::MenuItem("MenuState", NULL, typeid(*_state) == typeid(MenuState)))
					setState<MenuState>();
				if (ImGui::MenuItem("GameState", NULL, typeid(*_state) == typeid(GameState)))
					setState<GameState>();
				if (ImGui::MenuItem("EditorState", NULL, typeid(*_state) == typeid(EditorState)))
					setState<EditorState>();
				ImGui::EndMenu();
			}
			if (_state)
				_state->onMainMenu();
		}

		void setState_(std::unique_ptr<IState> state) final {
			_newState = std::move(state);
		}
		IState* getState() final { return _state.get(); }
		View::IView* getView() final { return _view.get(); }
		Renderer::IRenderer* getRenderer() final { return _renderer.get(); }
		Renderer::IUIRenderer* getUIRenderer() final { return _uiRenderer.get(); }

		void log(LogLevel level, const char* fmt, ...) {
			va_list va;
			va_start(va, fmt);
			vfprintf(stderr, fmt, va);
			fputc('\n', stderr);
			va_end(va);

			va_start(va, fmt);
			_uiRenderer->getLog()->log(level, fmt, va);
			va_end(va);
		}

	private:
		bool _quit;
		std::unique_ptr<View::IView> _view;
		std::unique_ptr<Renderer::IRenderer> _renderer;
		std::unique_ptr<Hydra::Renderer::IUIRenderer> _uiRenderer;

		std::unique_ptr<IState> _state;
		std::unique_ptr<IState> _newState;

		void _setupComponents() {
			using namespace Component::ComponentManager;
			auto& map = createOrGetComponentMap();
			registerComponents_graphics(map);
			registerComponents_network(map);
			registerComponents_physics(map);
			registerComponents_sound(map);
		}
	};
}

#undef main
int main(int argc, char** argv) {
	(void)argc;
	(void)argv;
	reportMemoryLeaks();
	Barcode::Engine engine;
	engine.setState<Barcode::MenuState>();
	engine.run();
	return 0;
}