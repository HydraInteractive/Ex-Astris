// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Everything related to loading and saving Blueprints.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/world/blueprintloader.hpp>

#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <hydra/world/world.hpp>

using namespace Hydra::World;

std::unique_ptr<Blueprint> BlueprintLoader::load(const std::string& file) {
	std::unique_ptr<Blueprint> bp;

	std::ifstream i(file);
	if (!i.good())
		return bp;

	bp = std::make_unique<Blueprint>();
	i >> bp->_root;

	bp->name = bp->_root["name"].get<std::string>();

	return bp;
}

void BlueprintLoader::save(const std::string& file, const std::string& name, std::shared_ptr<IEntity> entity) {
	std::ofstream o(file);
	if (!o.good())
		return;
	nlohmann::json root;

	root["name"] = name;
	entity->serialize(root["data"]);

	o << std::setw(2) << root << std::endl;
}
