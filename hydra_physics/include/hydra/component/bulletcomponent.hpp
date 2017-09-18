#pragma once
/**
* Player stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/

#pragma once
#include <hydra/ext/api.hpp>
#include <glm/glm.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/meshcomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API BulletComponent final : public IComponent{
	public:
		BulletComponent(IEntity* entity);
		BulletComponent(IEntity* entity, glm::vec3 position, glm::vec3 direction, float velocity);
		~BulletComponent() final;

		void tick(TickAction action) final;
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "BulletComponent"; }

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	private:
		glm::vec3 _position;
		glm::vec3 _direction;
		float _velocity;

	};
};
