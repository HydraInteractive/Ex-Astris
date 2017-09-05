/**
 * A function for getting the current stacktrace.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <string>

namespace Hydra::Ext {

	HYDRA_API std::string getStackTrace();

};
