#include <barcode/aiinspector.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/pathing/pathfinding.hpp>

AIInspector::AIInspector()
{
	testArray = new RGB[WORLD_MAP_SIZE*WORLD_MAP_SIZE];
	for (int x = 0; x < WORLD_MAP_SIZE; x++)
	{
		for (int y = 0; y < WORLD_MAP_SIZE; y++)
		{
			//Paint an X for testing on the map
			if (x == y || y == WORLD_MAP_SIZE - x - 1)
				testArray[x + (y*WORLD_MAP_SIZE)] = RGB{ 255, 255, 255 };
			else
				testArray[x + (y*WORLD_MAP_SIZE)] = RGB{ 0, 0, 0 };
		}
	}
}

AIInspector::~AIInspector()
{
}

void AIInspector::render(bool &openBool, Hydra::Component::TransformComponent* playerTransform)
{
	ImGui::SetNextWindowSize(ImVec2(1400, 700), ImGuiCond_Once);
	ImGui::Begin("AI Inspector", &openBool, ImGuiWindowFlags_MenuBar);
	_menuBar();
	if (_selectorMenuOpen)
	{
		ImGui::OpenPopup("AI Selector");
	}
	_aiSelector(playerTransform);
	if (!targetAI.expired() && testArray != nullptr)
	{
		ImGui::BeginChild("Interface", ImVec2(250, ImGui::GetWindowContentRegionMax().y - 60), true);
		ImGui::Checkbox("Trace Player", &_tracePlayer);
		ImGui::Checkbox("Trace AI", &_traceAI);
		ImGui::Separator();
		auto t = targetAI.lock()->getComponent<Hydra::Component::TransformComponent>();
		t->registerUI();
		ImGui::Separator();
		auto ai = targetAI.lock()->getComponent<Hydra::Component::AIComponent>();
		ai->registerUI();
		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::BeginChild("Pathing Map", ImVec2(ImGui::GetWindowContentRegionWidth() - 200, ImGui::GetWindowContentRegionMax().y - 60), true);
		if (_tracePlayer)
		{
			glm::ivec2 playerPos = PathFinding::worldToMapCoords(playerTransform->position).baseVec;
			testArray[playerPos.x + (playerPos.y * WORLD_MAP_SIZE)] = RGB{ 0, 255, 0 };
		}
		if (_traceAI)
		{
			glm::ivec2 aiPos = PathFinding::worldToMapCoords(targetAI.lock()->getComponent<Hydra::Component::TransformComponent>()->position).baseVec;
			testArray[aiPos.x + (aiPos.y * WORLD_MAP_SIZE)] = RGB{ 255, 0, 0 };
		}
		image = Hydra::Renderer::GLTexture::createFromData(WORLD_MAP_SIZE, WORLD_MAP_SIZE, Hydra::Renderer::TextureType::u8RGB, testArray);
		ImGui::Image((ImTextureID)image->getID(), ImVec2(WORLD_MAP_SIZE * 4, WORLD_MAP_SIZE * 4));

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
	else
	{
		ImGui::Text("No AI selected");
	}
	ImGui::End();
}

void AIInspector::_menuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Select AI", NULL))
			{
				_selectorMenuOpen = !_selectorMenuOpen;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

bool AIInspector::_aiSelector(Hydra::Component::TransformComponent* playerTransform)
{
	bool confirm = false;
	if (ImGui::BeginPopupModal("AI Selector", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string textline = "Select an AI to preview";
		ImGui::Text("%s", textline.c_str());

		ImGui::BeginChild("AI List", ImVec2(350, 500), true);
		std::string title = "";
		std::string pos = "";
		bool selected = false;

		std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
		Hydra::World::World::getEntitiesWithComponents<Hydra::Component::AIComponent, Hydra::Component::TransformComponent>(entities);
		for (auto e : entities)
		{
			auto t = e->getComponent<Hydra::Component::TransformComponent>();
			title = e->name + " [" + std::to_string(e->id) + "]";
			pos = " (" + std::to_string((int)t->position.x) + " " + std::to_string((int)t->position.y) + " " + std::to_string((int)t->position.z) + ")";
			if (!_selectedAI.expired())
				selected = (e == _selectedAI.lock());
			ImGui::MenuItem(title.c_str(), pos.c_str(), selected);
			if (ImGui::IsItemClicked())
			{
				_selectedAI = e;
			}
		}
		ImGui::EndChild();

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			targetAI = _selectedAI;
			confirm = true;
			_selectorMenuOpen = false;
			if (pathMap != nullptr)
			{
				for (int i = 0; i < WORLD_MAP_SIZE*WORLD_MAP_SIZE; i++)
				{
					if (pathMap[i] == true)
						testArray[i] = RGB{ 255, 255, 255 };
					else
						testArray[i] = RGB{ 0, 0, 0 };
				}
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			_selectorMenuOpen = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	return confirm;
}