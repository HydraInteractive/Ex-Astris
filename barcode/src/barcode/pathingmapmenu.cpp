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
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.2f));
	ImGui::Begin("Pathing", &closeBool, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	ImVec2 oldSpacing = ImGui::GetStyle().ItemSpacing;
	ImGui::GetStyle().ItemSpacing = ImVec2(0, 0);

	auto room = FileTree::getRoomEntity();
	auto r = room->getComponent<Hydra::Component::RoomComponent>();

	for (int i = 0; i < ROOM_MAP_SIZE; i++){
		for (int j = 0; j < ROOM_MAP_SIZE; j++){
			ImGui::PushID(i);
			ImGui::Selectable("", &r->localMap[i][j], 0, itemSize);
			if (ImGui::IsItemHovered() && ImGui::IsMouseDown(1))
				if (!ImGui::GetIO().KeyShift)
					r->localMap[i][j] = true;
				else
					r->localMap[i][j] = false;
			if (j != 31)ImGui::SameLine(0, 0);
			ImGui::PopID();
		}
	}

/*
	for (int i = 0; i < 32*32; i++)
	{
		ImGui::PushID(i);
		ImGui::Selectable("", &selected[i], 0, itemSize);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(1))
			if (!ImGui::GetIO().KeyShift)
				selected[i] = true;
			else
				selected[i] = false;

		if ((i % 32) < 31) ImGui::SameLine(0,0);
		ImGui::PopID();
	}*/


	/*ImGui::Columns(ROOM_MAP_SIZE);

	auto room = FileTree::getRoomEntity();
	auto r = room->getComponent<Hydra::Component::RoomComponent>();
	
	for (int i = 0; i < ROOM_MAP_SIZE; i++)
	{
		for (int j = 0; j < ROOM_MAP_SIZE; j++)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 10.0f);
			ImGui::BeginChild(j+(i*ROOM_MAP_SIZE), ImVec2(((ImGui::GetWindowContentRegionMax().x) / ROOM_MAP_SIZE) - 10.0f, ((ImGui::GetWindowContentRegionMax().y) / ROOM_MAP_SIZE - 4.5f)), true);

			ImGui::PushID(j);
			
			if(r->localMap[i][j] == 0)
			{ 
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(7.0f, 0.6f, 0.6f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(7.0f, 0.7f, 0.7f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(7.0f, 0.8f, 0.8f).Value);
				ImGui::SameLine(0.0f, 0.0f);

				if (ImGui::Button("", ImVec2(40, 15)))
				{
					r->localMap[i][j] = 1;
				}
			}
			else if (r->localMap[i][j] == 1)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(3 / 7.0f, 0.6f, 0.6f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(3 / 7.0f, 0.7f, 0.7f).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(3 / 7.0f, 0.8f, 0.8f).Value);
				ImGui::SameLine(0.0f, 0.0f);

				if (ImGui::Button("", ImVec2(40, 15)))
				{
					r->localMap[i][j] = 0;
				}
			}
			
			ImGui::PopStyleColor(3);
			ImGui::PopID();

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}
		ImGui::NextColumn();
	}*/
	ImGui::GetStyle().ItemSpacing = oldSpacing;
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}