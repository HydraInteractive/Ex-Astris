/**
* SpawnerComponent.
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#pragma once

#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/weaponcomponent.hpp>

using namespace Hydra::World;
namespace Hydra::Component {

	enum class SpawnerType {
		AlienSpawner = 0,
		RobotSpawner = 1,
	};

	struct HYDRA_PHYSICS_API SpawnerComponent final : public IComponent<SpawnerComponent, ComponentBits::Spawner>{
	SpawnerType spawnerID = SpawnerType::AlienSpawner;
	std::vector<int> spawnGroup = std::vector<int>();
	float spawnTimer = 0.0f;
	bool** map = nullptr;
	int spawnCounter = 0;
	glm::vec3 playerPos;

	~SpawnerComponent() final;

	inline const std::string type() const final { return "SpawnerComponent"; }
	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	void setTargetPlayer(std::shared_ptr<Hydra::World::Entity> player);
	};
};