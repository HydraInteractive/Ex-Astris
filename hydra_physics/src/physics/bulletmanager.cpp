// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Interface to manage the Bullet3 physics library.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/physics/bulletmanager.hpp>


class BulletManagerImpl final : public Hydra::Physics::IPhysicsManager {
public:
	BulletManagerImpl() {}
	~BulletManagerImpl() final {}

private:
};

std::unique_ptr<Hydra::Physics::IPhysicsManager> Hydra::Physics::BulletManager::create() {
	return std::unique_ptr<Hydra::Physics::IPhysicsManager>(new BulletManagerImpl());
}
