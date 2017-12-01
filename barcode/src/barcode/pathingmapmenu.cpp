#include <barcode/pathingmapmenu.hpp>

PathingMapMenu::PathingMapMenu()
{
}

PathingMapMenu::~PathingMapMenu()
{
}

void PathingMapMenu::render(bool & closeBool, float delta, int sizeX, int sizeY)
{
	ImGui::SetNextWindowSize(ImVec2(sizeX, sizeY), ImGuiCond_Once);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::Begin("Pathing", &closeBool, ImGuiWindowFlags_NoResize);
	ImGui::Columns(ROOM_MAP_SIZE);

	auto room = FileTree::getRoomEntity();
	auto r = room->getComponent<Hydra::Component::RoomComponent>();
	
	for (int i = 0; i < ROOM_MAP_SIZE; i++)
	{
		for (int j = 0; j < ROOM_MAP_SIZE; j++)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 10.0f);
			ImGui::BeginChild(j+(i*ROOM_MAP_SIZE), ImVec2(((ImGui::GetWindowContentRegionMax().x) / ROOM_MAP_SIZE) - 10.0f, ((ImGui::GetWindowContentRegionMax().y) / ROOM_MAP_SIZE - 4.5f)), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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
	}
	
	ImGui::End();
	ImGui::PopStyleColor();
}
