#include <hydra/engine.hpp>

#include <SDL2/SDL.h>
#include <hydra/view/sdlview.hpp>

#include <memory>
#include <ctime>
#include <hydra/component/componentmanager.hpp>
#include <hydra/component/componentmanager_network.hpp>
#include <hydra/component/componentmanager_graphics.hpp>
#include <hydra/component/componentmanager_physics.hpp>
#include <server/gameserver.hpp>
#include <hydra/engine.hpp>

#include <cstdio>
#include <chrono>

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

static inline void setup() {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}
#else
#include <signal.h>

static inline void setup() {
	signal(SIGPIPE, SIG_IGN);
}
#endif

static void onQuit() {
	// Mirror Hydra::World::World::reset, but without creating a new world root
	Hydra::World::World::_isResetting = true;
	Hydra::World::World::_entities.clear();
	Hydra::World::World::_map.clear();
}

using namespace Hydra;
namespace GServer {
	class Engine final : public IEngine {
	private:

	public:
		class Bogdan : IState {
		public:
			void* psystem = nullptr;
			void runFrame(float delta) {} void load() {} void onMainMenu() {} IO::ITextureLoader* getTextureLoader() { return nullptr; } IO::IMeshLoader* getMeshLoader() { return nullptr; } IO::ITextFactory* getTextFactory() { return nullptr; }
			Hydra::World::ISystem* getPhysicsSystem() { return (ISystem*)psystem; }
		};
		Bogdan _state;
		Engine() {
			IEngine::getInstance() = this;
			atexit(&onQuit);
		}

		~Engine() final {
			// Mirror Hydra::World::World::reset, but without creating a new world root
			Hydra::World::World::_isResetting = true;
			Hydra::World::World::_entities.clear();
			Hydra::World::World::_map.clear();
		}

		void run() final {}
		void quit() final {}

		void onMainMenu() final {}


		IState* getState() final { return (IState*)&_state; }
		void killme(void* psystem) {
			_state.psystem = psystem;
		}
		View::IView* getView() final { return nullptr; } Renderer::IRenderer* getRenderer() final { return nullptr; } Renderer::IUIRenderer* getUIRenderer() final { return nullptr; } Hydra::System::DeadSystem* getDeadSystem() final { return nullptr; }void setState_(std::unique_ptr<IState> state) final {}
		void log(LogLevel level, const char* fmt, ...) {
			va_list va;
			va_start(va, fmt);
#ifdef __linux__
			static const char* color[] = { "\x1b[39;1m", "\x1b[33;1m", "\x1b[31;1m", "\x1b[37;41;1m" };
			fputs(color[static_cast<int>(level)], stderr);
#endif
			vfprintf(stderr, fmt, va);
#ifdef __linux__
			fputs("\x1b[0m", stderr);
#endif
			fputc('\n', stderr);
			va_end(va);
		}
	};
}
#undef main
int main(int argc, char** argv) {
	srand(time(NULL));
	(void)argc;
	(void)argv;
	setup();
	SDLNet_Init();
	using namespace Hydra::Component::ComponentManager;
	auto& map = createOrGetComponentMap();
	GServer::Engine engine;
	
	registerComponents_graphics(map);
	registerComponents_network(map);
	registerComponents_physics(map);
	//registerComponents_sound(map);
	BarcodeServer::GameServer server;
	((GServer::Engine::Bogdan*)engine.getState())->psystem = (void*)(&server._physicsSystem);
	if (server.initialize(4545)) {
		Hydra::World::World::reset();
		server.start();
		while (true) {
			server.run();
		}
	}
	return 0;
}
