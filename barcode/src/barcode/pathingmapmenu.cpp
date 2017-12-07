#include <barcode/pathingmapmenu.hpp>

PathingMapMenu::PathingMapMenu()
{
}

PathingMapMenu::~PathingMapMenu()
{
}

void PathingMapMenu::render(bool & closeBool, float delta, int sizeX, int sizeY)
{
	ImVec2 itemSize(sizeX / 32, (sizeY - 24) / 32);
	ImVec2 startPadding(-8, 16);

	ImGui::SetNextWindowSize(ImVec2(sizeX+20, sizeY));
	ImGui::SetNextWindowPos(startPadding);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.0f, 0.0f, 0.0f, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.2f));
	ImGui::Begin("Pathing", &closeBool, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	ImVec2 oldSpacing = ImGui::GetStyle().ItemSpacing;
	ImGui::GetStyle().ItemSpacing = ImVec2(0, 0);

	auto room = FileTree::getRoomEntity();
	auto r = room->getComponent<Hydra::Component::RoomComponent>();

	bool reversedPos;

	for (size_t i = 0; i < ROOM_MAP_SIZE; i++){
		for (size_t j = 0; j < ROOM_MAP_SIZE; j++){
			ImGui::PushID(i*ROOM_MAP_SIZE+j);
			reversedPos = !r->localMap[j][i];

			if (ImGui::Selectable("", &reversedPos, 0, itemSize))
				r->localMap[j][i] = !r->localMap[j][i];

			if (ImGui::IsItemHovered() && ImGui::IsMouseDown(1)) {
				if (ImGui::GetIO().KeyShift)
					r->localMap[j][i] = true;
				else
					r->localMap[j][i] = false;
			}

			if (j != 31)
				ImGui::SameLine(0, 0);
			ImGui::PopID();
		}
	}

	ImGui::GetStyle().ItemSpacing = oldSpacing;
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}
