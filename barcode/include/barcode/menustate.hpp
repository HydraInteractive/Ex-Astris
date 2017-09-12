/**
 * Mainmenu test state
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once

#include <hydra/engine.hpp>

class MenuState final : public IState {
public:
	~MenuState() final {
	}

	inline static IState* getInstance() {
		static MenuState instance;
		return &instance;
	}

	void onEnter(IState* oldState) final {
	}

	void runFrame() final {
	}

	void onLeave(IState* newState) final {
	}
private:
};
