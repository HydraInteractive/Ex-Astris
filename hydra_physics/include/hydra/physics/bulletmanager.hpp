/**
 * Interface to manage the Bullet3 physics library.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <hydra/physics/physicsmanager.hpp>
#include <memory>

namespace Hydra::Physics {
	namespace BulletManager {
		HYDRA_API std::unique_ptr<IPhysicsManager> create();
	}
}
