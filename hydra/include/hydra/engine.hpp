#pragma once

#include "singleton.hpp"

namespace Hydra {
class Engine : public Singleton<Engine> {
public:
	void run();

private:
	friend class Singleton;
	Engine();
	virtual ~Engine();
};
}
