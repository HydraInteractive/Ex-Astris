#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <memory>

using namespace Hydra::World;

class AIInspector final {
public:
	std::weak_ptr<Hydra::World::Entity> targetAI = std::weak_ptr<Hydra::World::Entity>();

	AIInspector();
	~AIInspector();

	void render(bool &openBool);
private:
	bool _selectorMenuOpen = false;
	std::weak_ptr<Hydra::World::Entity> _selectedAI = std::weak_ptr<Hydra::World::Entity>();
	void _menuBar();
	bool _aiSelector();
};