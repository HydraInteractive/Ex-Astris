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

#include <json.hpp>
using namespace Hydra::World;

std::unique_ptr<Blueprint> BlueprintLoader::load(const std::string& file) {
	std::unique_ptr<Blueprint> bp;
	std::vector<uint8_t> d;

	//printf("Loading blueprint: %s\n", file.c_str());
	FILE* fp = fopen(file.c_str(), "rb");
	fseek(fp, 0, SEEK_END);
	d.resize(ftell(fp));
	fseek(fp, 0, SEEK_SET);
	fread(d.data(), d.size(), 1, fp);
	fclose(fp);

	bp = std::make_unique<Blueprint>();
	bp->_root = nlohmann::json::from_msgpack(d);

	bp->name = bp->_root["name"].get<std::string>();

	return bp;
}

void BlueprintLoader::save(const std::string& file, const std::string& name, Entity* entity) {
	nlohmann::json root;

	printf("Saving blueprint: %s\n", file.c_str());
	root["name"] = name;
	entity->serialize(root["data"]);

	FILE* fp = fopen(file.c_str(), "wb");
	auto d = nlohmann::json::to_msgpack(root);
	fwrite(d.data(), d.size(), 1, fp);
	fclose(fp);
}
