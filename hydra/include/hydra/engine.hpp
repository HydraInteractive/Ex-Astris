#pragma once

namespace Hydra {
	class Engine {
	public:
		virtual ~Engine() = 0;

		virtual void run() = 0;
	};
	inline Engine::~Engine() {}
}
