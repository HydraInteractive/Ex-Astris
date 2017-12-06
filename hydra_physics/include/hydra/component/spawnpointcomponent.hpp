#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <hydra/world/world.hpp>
#include <hydra/ext/api.hpp>
using namespace Hydra::World;
namespace Hydra::Component {
	struct HYDRA_PHYSICS_API SpawnPointComponent final : public IComponent<SpawnPointComponent, ComponentBits::SpawnPoint>{
		~SpawnPointComponent() final;

		bool playerSpawn = false;
		bool enemySpawn = false;

		inline const std::string type() const final { return "SpawnPointComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};