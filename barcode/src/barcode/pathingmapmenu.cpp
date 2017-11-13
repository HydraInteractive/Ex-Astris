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
	for (int i = 0; i < ROOM_MAP_SIZE; i++)
	{
		for (int j = 0; j < ROOM_MAP_SIZE; j++)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
			ImGui::BeginChild(j+(i*ROOM_MAP_SIZE), ImVec2(((ImGui::GetWindowContentRegionMax().x) / ROOM_MAP_SIZE) - 20.0f, ((ImGui::GetWindowContentRegionMax().y) / ROOM_MAP_SIZE - 10.0f)), true);

			ImGui::EndChild();
			ImGui::PopStyleVar();
		}
		ImGui::NextColumn();
	}
	

	ImGui::End();
}