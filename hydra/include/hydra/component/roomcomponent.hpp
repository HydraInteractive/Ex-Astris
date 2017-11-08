#pragma once

#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/icons.hpp>

#include <hydra/ext/api.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_BASE_API RoomComponent final : public IComponent<RoomComponent, ComponentBits::Room>
	{
		enum { NORTH, EAST, SOUTH, WEST };
		bool open[4];
		bool door[4];
		bool change = 0;
		~RoomComponent() final;

		inline const std::string type() const final { return "RoomComponent"; }
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};