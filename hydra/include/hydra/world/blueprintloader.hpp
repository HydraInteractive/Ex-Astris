/**
 * Everything related to loading and saving Blueprints.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>

namespace Hydra::World {
	namespace BlueprintLoader {
		HYDRA_BASE_API std::unique_ptr<Blueprint> load(const std::string& file);
		HYDRA_BASE_API void save(const std::string& file, const std::string& name, Entity* entity);
	};
};


/*

#1: Map<3> {
  'Name': string
  'Components': Map<'ComponentName' string, Map<'FieldName' string, T>> )
  'Children': Array<*>( "Go to #1" )
}

 */
