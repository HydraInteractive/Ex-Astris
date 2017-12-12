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

#define ROOM_GRID_SIZE 9 //Size of the 2D array holding all rooms
#define ROOM_SIZE 34 //How long a side of a room is
#define ROOM_MAP_SIZE 32 //Size of both dimensions of the 2D array
#define ROOM_SCALE (ROOM_SIZE / ROOM_MAP_SIZE) //How map coordinates relate to world coordinates
#define WORLD_MAP_SIZE (ROOM_GRID_SIZE * ROOM_MAP_SIZE) //Size of the 2D array holding the pathing map for the whole level

using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_BASE_API RoomComponent final : public IComponent<RoomComponent, ComponentBits::Room>
	{
		enum { NORTH, EAST, SOUTH, WEST };
		bool openWalls[4] = { 0 };
		bool door[4] = { 0 };
		bool localMap[ROOM_MAP_SIZE][ROOM_MAP_SIZE] = { 0 };
		bool change = 0;
		glm::ivec2 gridPosition;

		~RoomComponent() final;

		inline const std::string type() const final { return "RoomComponent"; }
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	};
};