#pragma once

#include <hydra/world/world.hpp>

namespace Hydra::World {
	namespace BlueprintLoader {
		std::unique_ptr<Blueprint> load(const std::string& file);
		void save(const std::string& file, const std::string& name, std::shared_ptr<IEntity> entity);
	};
};


/*

#1: Map<3> {
  'Name': string
  'Components': Map<'ComponentName' string, Map<'FieldName' string, T>> )
  'Children': Array<*>( "Go to #1" )
}

 */
