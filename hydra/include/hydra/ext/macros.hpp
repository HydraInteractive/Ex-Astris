/**
 * Helper macros for doing stuff
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once

#define ABSTRACT_STRUCT(name)	struct HYDRA_API name { virtual ~name() = 0; }; inline name::~name() {}

namespace Hydra::Ext {
	template <class... Args>
	struct TypeTuple {};
}
