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

	json["openN"] = openWalls[NORTH];
	json["openE"] = openWalls[EAST];
	json["openS"] = openWalls[SOUTH];
	json["openW"] = openWalls[WEST];
	
	json["mapSize"] = (unsigned int)ROOM_MAP_SIZE;
	for (size_t i = 0; i < ROOM_MAP_SIZE; i++)
	{
		for (size_t j = 0; j < ROOM_MAP_SIZE; j++)
		{
			json["map"][i][j] = localMap[i][j];
		}
	}
}

void Hydra::Component::RoomComponent::deserialize(nlohmann::json & json)
{
	door[NORTH] = json.value<bool>("doorsN", false);
	door[EAST] = json.value<bool>("doorsE", false);
	door[SOUTH] = json.value<bool>("doorsS", false);
	door[WEST] = json.value<bool>("doorsW", false);

	openWalls[NORTH] = json.value<bool>("openN", false);
	openWalls[EAST] = json.value<bool>("openE", false);
	openWalls[SOUTH] = json.value<bool>("openS", false);
	openWalls[WEST] = json.value<bool>("openW", false);

	if (json.value<unsigned int>("mapSize", 0) == ROOM_MAP_SIZE)
	{
		for (size_t i = 0; i < ROOM_MAP_SIZE; i++)
		{
			for (size_t j = 0; j < ROOM_MAP_SIZE; j++)
			{
				localMap[i][j] = json["map"][i][j].get<bool>();
			}
		}
	}
	else
	{
		for (size_t i = 0; i < ROOM_MAP_SIZE; i++)
		{
			for (size_t j = 0; j < ROOM_MAP_SIZE; j++)
			{
				localMap[i][j] = true;
			}
		}
	}
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
	ImGui::Checkbox("North", &openWalls[NORTH]);
	ImGui::SameLine();
	ImGui::Checkbox("East", &openWalls[EAST]);
	ImGui::SameLine();
	ImGui::Checkbox("South", &openWalls[SOUTH]);
	ImGui::SameLine();
	ImGui::Checkbox("West", &openWalls[WEST]);
}
