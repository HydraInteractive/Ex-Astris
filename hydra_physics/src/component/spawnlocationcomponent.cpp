#include <hydra/component/spawnlocationcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <imgui/imgui.h>
using namespace Hydra::World;
using namespace Hydra::Component;

SpawnLocationComponent::~SpawnLocationComponent()
{

}

void Hydra::Component::SpawnLocationComponent::serialize(nlohmann::json& json) const
{
	json["playerSpawn"] = playerSpawn;
	json["enemySpawn"] = enemySpawn;
}

void Hydra::Component::SpawnLocationComponent::deserialize(nlohmann::json& json)
{
	playerSpawn = json.value<bool>("playerSpawn", false);
	enemySpawn = json.value<bool>("enemySpawn", false);
}

void Hydra::Component::SpawnLocationComponent::registerUI()
{
	ImGui::Checkbox("Spawn Players", &playerSpawn);
	ImGui::Checkbox("Spawn Enemies", &enemySpawn);
}
