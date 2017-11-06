/**
* stuff used in movement
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#include <hydra/ext/api.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <hydra/world/world.hpp>


using namespace Hydra::World;

namespace Hydra::Component {
	struct HYDRA_PHYSICS_API MovementComponent final : public IComponent<MovementComponent, ComponentBits::Movement>{
	glm::vec3 direction = glm::vec3{0, 0, 0};
	glm::vec3 velocity = glm::vec3{0, 0, 0};
	glm::vec3 acceleration = glm::vec3{0, 0, 0};
	float movementSpeed = 20.0f;

	~MovementComponent() final;

	inline const std::string type() const final { return "MovementComponent"; }

	void serialize(nlohmann::json& json) const final;
	void deserialize(nlohmann::json& json) final;
	void registerUI() final;
	};
};

