#include <hydra/engine.hpp>

#include <SDL2/SDL.h>

#include <memory>

#include <hydra/component/componentmanager.hpp>
#include <hydra/component/componentmanager_network.hpp>
//#include <hydra/component/componentmanager_graphics.hpp>
#include <hydra/component/componentmanager_physics.hpp>
#include <server/gameserver.hpp>

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

#undef main
int main(int argc, char** argv) {
	(void)argc;
	(void)argv;
	setup();
	SDLNet_Init();
	using namespace Hydra::Component::ComponentManager;
	auto& map = createOrGetComponentMap();
	//registerComponents_graphics(map);
	registerComponents_network(map);
	registerComponents_physics(map);
	//registerComponents_sound(map);
	GameServer server;
	if (server.initialize(4545)) {
		World::reset();
		server.start();
		while (true) {
			server.run();
		}
	}
	return 0;
}
