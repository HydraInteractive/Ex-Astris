/**
 * Interface to manage a physics system.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <cstddef>
#include <hydra/ext/macros.hpp>

namespace Hydra::Physics {
	// Network will probably overwrite this to easily know what data to send.
	struct HYDRA_API IMotionState {
		virtual ~IMotionState() = 0;

		typedef void (*Callback)(void* userptr);

		// Should hopefully only be used by the server
		virtual void setCallback(Callback cb, void* userptr) = 0;
	};
	inline IMotionState::~IMotionState() {}

	enum class CollisionShape {
		box
	};
	ABSTRACT_STRUCT(IShape);

	class HYDRA_API IPhysicsManager {
	public:
		virtual ~IPhysicsManager() = 0;

	};
	inline IPhysicsManager::~IPhysicsManager() {}
}
