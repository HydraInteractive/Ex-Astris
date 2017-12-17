#include <barcode/minimap.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/pathing/pathfinding.hpp>
#include <hydra/network/packets.hpp>
#include <hydra/network/netclient.hpp>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL.h>
Hydra::Component::TransformComponent* MiniMap::playerTransform = nullptr;
MiniMap::MiniMap()
{
	reset();
}

MiniMap::~MiniMap()
{
}

void MiniMap::render(bool &openBool, Hydra::Component::TransformComponent* _playerTransform, glm::ivec2 windowSize)
{
	playerTransform = _playerTransform;
	//const Uint8* keysArray = SDL_GetKeyboardState(nullptr);
	float scale = 1.0f;
	//if (keysArray[SDL_SCANCODE_H])
	//	scale = 2;
	ImGui::SetNextWindowSize(ImVec2(WORLD_MAP_SIZE*scale, WORLD_MAP_SIZE*scale), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(windowSize.x - WORLD_MAP_SIZE*scale, windowSize.y - WORLD_MAP_SIZE*scale));
	std::cout << windowSize.x << " " << windowSize.y << std::endl;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	
	ImGui::Begin("MiniMap", &openBool, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	if (colourArray != nullptr)
	{
		glm::ivec2 playerPos = PathFinding::worldToMapCoords(playerTransform->position).baseVec;
		colourArray[playerPos.x + (playerPos.y * WORLD_MAP_SIZE)] = RGB{ 0, 255, 0 };
		
		image = Hydra::Renderer::GLTexture::createFromData(WORLD_MAP_SIZE, WORLD_MAP_SIZE, Hydra::Renderer::TextureType::u8RGB, colourArray);
		ImGui::Image(reinterpret_cast<ImTextureID>(image->getID()), ImVec2(WORLD_MAP_SIZE*scale, WORLD_MAP_SIZE*scale));
	}
	else
	{
		ImGui::Text("No AI selected");
	}
	ImGui::PopStyleVar();
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void MiniMap::reset()
{
	if (colourArray == nullptr)
	{
		colourArray = new RGB[WORLD_MAP_SIZE*WORLD_MAP_SIZE];
	}
	if (pathMap != nullptr)
	{
		for (int i = 0; i < WORLD_MAP_SIZE*WORLD_MAP_SIZE; i++)
		{
			if (pathMap[i])
				colourArray[i] = RGB{ 255, 255, 255 };
			else
				colourArray[i] = RGB{ 0, 0, 0 };
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
					colourArray[x + (y*WORLD_MAP_SIZE)] = RGB{ 255, 255, 255 };
				else
					colourArray[x + (y*WORLD_MAP_SIZE)] = RGB{ 0, 0, 0 };
			}
		}
	}
}