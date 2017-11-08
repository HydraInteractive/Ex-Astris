#include <hydra/component/roomcomponent.hpp>

Hydra::Component::RoomComponent::~RoomComponent()
{

}

void Hydra::Component::RoomComponent::serialize(nlohmann::json & json) const
{
	json["doorsN"] = door[NORTH];
	json["doorsE"] = door[EAST];
	json["doorsS"] = door[SOUTH];
	json["doorsW"] = door[WEST];

	json["openN"] = open[NORTH];
	json["openE"] = open[EAST];
	json["openS"] = open[SOUTH];
	json["openW"] = open[WEST];
}

void Hydra::Component::RoomComponent::deserialize(nlohmann::json & json)
{
	door[NORTH] = json.value<bool>("doorsN", false);
	door[EAST] = json.value<bool>("doorsE", false);
	door[SOUTH] = json.value<bool>("doorsS", false);
	door[WEST] = json.value<bool>("doorsW", false);

	open[NORTH] = json.value<bool>("openN", false);
	open[EAST] = json.value<bool>("openE", false);
	open[SOUTH] = json.value<bool>("openS", false);
	open[WEST] = json.value<bool>("openW", false);
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
