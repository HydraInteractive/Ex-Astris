#include <hydra/component/spawnpointcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <imgui/imgui.h>
using namespace Hydra::World;
using namespace Hydra::Component;

SpawnPointComponent::~SpawnPointComponent()
{

}

void Hydra::Component::SpawnPointComponent::serialize(nlohmann::json& json) const
{
	json["playerSpawn"] = playerSpawn;
	json["enemySpawn"] = enemySpawn;
}

void Hydra::Component::SpawnPointComponent::deserialize(nlohmann::json& json)
{
	playerSpawn = json.value<bool>("playerSpawn", false);
	enemySpawn = json.value<bool>("enemySpawn", false);
}

void Hydra::Component::SpawnPointComponent::registerUI()
{
	ImGui::Checkbox("Spawn Players", &playerSpawn);
	ImGui::Checkbox("Spawn Enemies", &enemySpawn);
}
