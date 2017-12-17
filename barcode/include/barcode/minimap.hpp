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
class MiniMap final {
public:
	struct RGB
	{
		uint8_t r, g, b;
	};
	RGB* colourArray = nullptr;
	static Hydra::Component::TransformComponent* playerTransform;
	bool* pathMap = nullptr;
	std::shared_ptr<ITexture> image = nullptr;

	MiniMap();
	~MiniMap();

	void render(bool &openBool, Hydra::Component::TransformComponent* _playerTransform, glm::ivec2 windowSize);
	void reset();
private:
};