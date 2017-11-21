#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hydra/world/world.hpp>
#include <hydra/ext/api.hpp>
using namespace Hydra::World;
namespace Hydra::Component {
	struct HYDRA_PHYSICS_API SpawnLocationComponent final : public IComponent<SpawnLocationComponent, ComponentBits::SpawnLocation>{
		~SpawnLocationComponent() final;

		bool playerSpawn = true;
		bool enemySpawn = true;

		inline const std::string type() const final { return "SpawnLocationComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};