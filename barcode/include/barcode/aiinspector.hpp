#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <memory>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <hydra/engine.hpp>
#include <hydra/renderer/renderer.hpp>
#include <hydra/renderer/glrenderer.hpp>
#include <hydra/component/transformcomponent.hpp>
using namespace Hydra::World;
class AIInspector final {
public:
	struct RGB
	{
		uint8_t r, g, b;
	};
	static Hydra::Component::TransformComponent* playerTransform;
	typedef void(*getAIInfo_f)(void* userdata);
	static getAIInfo_f getAIInfo;
	static void* userdata;

	std::weak_ptr<Hydra::World::Entity> targetAI = std::weak_ptr<Hydra::World::Entity>();
	RGB* colourArray = nullptr;
	bool* pathMap = nullptr;
	std::shared_ptr<ITexture> image = nullptr;

	AIInspector();
	~AIInspector();

	void render(bool &openBool, Hydra::Component::TransformComponent* _playerTransform);
	void updatePath(std::vector<glm::ivec2>& openList, std::vector<glm::ivec2>& closedList, std::vector<glm::ivec2>& pathToEnd);
	void reset();
private:
	struct {
		bool operator()(const std::shared_ptr<Hydra::World::Entity>& left, const std::shared_ptr<Hydra::World::Entity>& right) const
		{
			auto leftT = left->getComponent<Hydra::Component::TransformComponent>();
			auto rightT = right->getComponent<Hydra::Component::TransformComponent>();

			return glm::length(leftT->position - playerTransform->position) < glm::length(rightT->position - playerTransform->position);
		}
	} _distanceToPlayerComparator;
	bool _selectorMenuOpen = false;
	bool _tracePlayer = true;
	bool _traceAI = true;
	bool _smallMap = false;
	bool _showOptions = true;
	std::weak_ptr<Hydra::World::Entity> _selectedAI = std::weak_ptr<Hydra::World::Entity>();
	void _menuBar();
	bool _aiSelector();
};