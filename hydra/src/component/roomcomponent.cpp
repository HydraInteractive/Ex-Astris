#include <hydra/component/roomcomponent.hpp>

Hydra::Component::RoomComponent::~RoomComponent()
{

}

void Hydra::Component::RoomComponent::serialize(nlohmann::json & json) const
{
	json = {
		{ "doors",{ door[NORTH], door[EAST], door[SOUTH], door[WEST] } },
		{ "openWalls",{ open[NORTH], open[EAST], open[SOUTH], open[WEST] } }
	};
}

void Hydra::Component::RoomComponent::deserialize(nlohmann::json & json)
{
	auto& doors = json["doors"];
	door[NORTH] = doors[NORTH].get<bool>();
	door[EAST] = doors[EAST].get<bool>();
	door[SOUTH] = doors[SOUTH].get<bool>();
	door[WEST] = doors[WEST].get<bool>();

	auto& openWalls = json["openWalls"];
	open[NORTH] = openWalls[NORTH].get<bool>();
	open[EAST] = openWalls[EAST].get<bool>();
	open[SOUTH] = openWalls[SOUTH].get<bool>();
	open[WEST] = openWalls[WEST].get<bool>();
}

void Hydra::Component::RoomComponent::registerUI()
{
	ImGui::Text("Doors:");
	ImGui::Checkbox("North", &door[NORTH]);
	ImGui::SameLine();
	ImGui::Checkbox("East", &door[EAST]);
	ImGui::SameLine();
	ImGui::Checkbox("South", &door[SOUTH]);
	ImGui::SameLine();
	ImGui::Checkbox("West", &door[WEST]);

	ImGui::Text("Open Walls:");
	ImGui::Checkbox("North", &door[NORTH]);
	ImGui::SameLine();
	ImGui::Checkbox("East", &door[EAST]);
	ImGui::SameLine();
	ImGui::Checkbox("South", &door[SOUTH]);
	ImGui::SameLine();
	ImGui::Checkbox("West", &door[WEST]);
}
