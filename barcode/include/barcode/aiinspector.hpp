#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <memory>
#include <imgui/imgui.h>
using namespace Hydra::World;

class AIInspector final {
public:
	std::weak_ptr<Hydra::World::Entity> targetAI = std::weak_ptr<Hydra::World::Entity>();
	ImColor** testArray = nullptr;
	AIInspector();
	~AIInspector();

	void render(bool &openBool);
private:
	const int sizeX = 16;
	const int sizeY = 16;
	bool _selectorMenuOpen = false;
	std::weak_ptr<Hydra::World::Entity> _selectedAI = std::weak_ptr<Hydra::World::Entity>();
	void _menuBar();
	bool _aiSelector();
};