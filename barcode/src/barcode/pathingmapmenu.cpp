#include "..\..\include\barcode\pathingmapmenu.hpp"

PathingMapMenu::PathingMapMenu()
{
}

PathingMapMenu::~PathingMapMenu()
{
}

void PathingMapMenu::render(bool & closeBool, float delta)
{
	ImGui::SetNextWindowSize(ImVec2(960, 960), ImGuiSetCond_Once);
	ImGui::Begin("Pathing", &closeBool);
	ImGui::Columns(ROOM_MAP_SIZE);

	auto room = FileTree::getRoomEntity();
	auto r = room->getComponent<Hydra::Component::RoomComponent>();
	
	for (int i = 0; i < ROOM_MAP_SIZE; i++)
	{
		for (int j = 0; j < ROOM_MAP_SIZE; j++)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
			ImGui::BeginChild(j+(i*ROOM_MAP_SIZE), ImVec2(((ImGui::GetWindowContentRegionMax().x) / ROOM_MAP_SIZE) - 20.0f, ((ImGui::GetWindowContentRegionMax().y) / ROOM_MAP_SIZE - 10.0f)), true);

			ImGui::PushID(j);
			
			if(r->localMap[i][j] == 0)
			{ 
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(3/7.0f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(3/7.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(3 / 7.0f, 0.8f, 0.8f));
			
				if (ImGui::Button("", ImVec2(25, 35)))
				{
					r->localMap[i][j] = 1;
				}
			}
			else if (r->localMap[i][j] == 1)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(7.0f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(7.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(7.0f, 0.8f, 0.8f));
				if (ImGui::Button("", ImVec2(25, 35)))
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
	}
	

	ImGui::End();
}