#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <memory>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <hydra/engine.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/renderer/glrenderer.hpp>
using namespace Hydra::World;

class AIInspector final {
public:
	struct RGB
	{
		uint8_t r, g, b;
	};
	std::weak_ptr<Hydra::World::Entity> targetAI = std::weak_ptr<Hydra::World::Entity>();
	RGB* testArray = nullptr;
	bool* pathMap = nullptr;
	std::shared_ptr<ITexture> image = nullptr;
	AIInspector();
	~AIInspector();

	void render(bool &openBool, Hydra::Component::TransformComponent* playerTransform);
	void reset();
private:
	bool _selectorMenuOpen = false;
	bool _tracePlayer = true;
	bool _traceAI = true;
	std::weak_ptr<Hydra::World::Entity> _selectedAI = std::weak_ptr<Hydra::World::Entity>();
	void _menuBar();
	bool _aiSelector(Hydra::Component::TransformComponent* playerTransform);
};