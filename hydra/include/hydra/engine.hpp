#pragma once

namespace Hydra {
	class Engine {
	public:
		static Engine& getInstance();

		virtual ~Engine() = 0;

		virtual void run() = 0;
	};
	inline Engine::~Engine() {}
}
