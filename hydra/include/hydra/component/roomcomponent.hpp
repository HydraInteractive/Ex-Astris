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

#define GRID_SIZE 7 //The number of rooms in x and y of the whole map
#define ROOM_SIZE 34
#define ROOM_MAP_SIZE 16 //The size in both X and Z of one local room map
#define ROOM_SCALE (ROOM_SIZE / ROOM_MAP_SIZE)
#define FULL_MAP_SIZE (GRID_SIZE * ROOM_MAP_SIZE)

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_BASE_API RoomComponent final : public IComponent<RoomComponent, ComponentBits::Room>
	{
		enum { NORTH, EAST, SOUTH, WEST };
		bool openWalls[4];
		bool door[4];
		int localMap[ROOM_MAP_SIZE][ROOM_MAP_SIZE];
		bool change = 0;
		~RoomComponent() final;

		inline const std::string type() const final { return "RoomComponent"; }
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};