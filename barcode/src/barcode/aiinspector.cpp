#include <barcode/aiinspector.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/network/packets.hpp>
#include <hydra/network/netclient.hpp>
AIInspector::getAIInfo_f AIInspector::getAIInfo = nullptr;
void* AIInspector::userdata = nullptr;

AIInspector::AIInspector()
{
	reset();
}

AIInspector::~AIInspector()
{
}

void AIInspector::render(bool &openBool, Hydra::Component::TransformComponent* _playerTransform)
{
	playerTransform = _playerTransform;
	ImGui::SetNextWindowSize(ImVec2(1400, 700), ImGuiCond_Once);
	ImGui::Begin("AI Inspector", &openBool, ImGuiWindowFlags_MenuBar);
	_menuBar();
	if (_selectorMenuOpen)
	{
		ImGui::OpenPopup("AI Selector");
	}
	_aiSelector();
	if (!targetAI.expired() && testArray != nullptr)
	{
		if (_showOptions)
		{
			ImGui::BeginChild("Interface", ImVec2(250, ImGui::GetWindowContentRegionMax().y - 60), true);

			ImGui::Checkbox("Trace Player", &_tracePlayer);
			ImGui::Checkbox("Trace AI", &_traceAI);
			if (ImGui::Button("Reset"))
			{
				reset();
			}
			ImGui::Separator();

			auto t = targetAI.lock()->getComponent<Hydra::Component::TransformComponent>();
			t->registerUI();
			ImGui::Separator();

			auto ai = targetAI.lock()->getComponent<Hydra::Component::AIComponent>();
			ai->registerUI();

			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::BeginChild("Pathing Map", ImVec2(ImGui::GetWindowContentRegionWidth() - 250, ImGui::GetWindowContentRegionMax().y - 60), true);
		}
		else
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::BeginChild("Pathing Map", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowContentRegionMax().y - 60), true);
		}
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
		int scale = 1;
		if (!_smallMap)
			scale = 4;
		ImGui::Image(reinterpret_cast<ImTextureID>(image->getID()), ImVec2(WORLD_MAP_SIZE * scale, WORLD_MAP_SIZE * scale));

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
	else
	{
		ImGui::Text("No AI selected");
	}
	ImGui::End();
}

void AIInspector::reset()
{
	if (testArray == nullptr)
	{
		testArray = new RGB[WORLD_MAP_SIZE*WORLD_MAP_SIZE];
	}
	if (pathMap != nullptr)
	{
		for (int i = 0; i < WORLD_MAP_SIZE*WORLD_MAP_SIZE; i++)
		{
			if (pathMap[i])
				testArray[i] = RGB{ 255, 255, 255 };
			else
				testArray[i] = RGB{ 0, 0, 0 };
		}
	}
	else
	{
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
			if (ImGui::MenuItem("Small map", NULL, _smallMap))
			{
				_smallMap = !_smallMap;
			}
			if (ImGui::MenuItem("Show options", NULL, _showOptions))
			{
				_showOptions = !_showOptions;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

bool AIInspector::_aiSelector()
{
	bool confirm = false;
	if (ImGui::BeginPopupModal("AI Selector", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string textline = "Select an AI to preview";
		ImGui::Text("%s", textline.c_str());

		ImGui::BeginChild("AI List", ImVec2(400, 600), true);
		std::string title = "";
		std::string pos = "";
		bool selected = false;

		std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
		Hydra::World::World::getEntitiesWithComponents<Hydra::Component::AIComponent, Hydra::Component::TransformComponent>(entities);
		std::sort(entities.begin(), entities.end(), _distanceToPlayerComparator);
		for (auto e : entities)
		{
			auto t = e->getComponent<Hydra::Component::TransformComponent>();
			title = e->name + " [" + std::to_string(e->id) + "]";
			int distance = glm::length(t->position - playerTransform->position);
			pos = " (" + std::to_string((int)t->position.x) + " " + std::to_string((int)t->position.y) + " " + std::to_string((int)t->position.z) + ") Distance: " + std::to_string(distance);
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
				reset();
			}
			Hydra::Network::NetClient::requestAIInfo(targetAI.lock()->id);
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