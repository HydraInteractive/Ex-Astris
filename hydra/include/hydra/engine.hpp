#pragma once

namespace Hydra {
	class Engine {
	public:
		static Engine& getInstance();
		
		virtual void run() = 0;
	};
}
