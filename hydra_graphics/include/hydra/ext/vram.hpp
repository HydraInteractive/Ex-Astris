/**
 * Functions for getting the amount of VRAM that Hydra is using.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <string>

namespace Hydra::Ext {
	HYDRA_API size_t getCurrentVRAM();
	HYDRA_API size_t getMaxVRAM();
	HYDRA_API bool isVRAMDedicated();
};
