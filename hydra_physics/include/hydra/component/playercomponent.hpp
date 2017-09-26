#pragma once
/**
* Player stuff
*
* License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
* Authors:
*  - Dan Printzell
*/
#include <hydra/ext/api.hpp>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/bulletcomponent.hpp>
#include <hydra/component/grenadecomponent.hpp>

using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API PlayerComponent final : public IComponent{
	public:
		PlayerComponent(IEntity* entity);
		~PlayerComponent() final;

		void tick(TickAction action, float delta) final;
		// If you want to add more than one TickAction, combine them with '|' (The bitwise or operator) 
		inline TickAction wantTick() const final { return TickAction::physics; }

		std::shared_ptr<Hydra::World::IEntity> getWeapon();

		inline const std::string type() const final { return "PlayerComponent"; }
		const glm::vec3 getPosition() { return _position; };
		void throwGrenade();

		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;
	private:
		glm::vec3 _position = glm::vec3(0,-2,3);
		glm::vec3 _velocity;
		glm::vec3 _acceleration;
		float _movementSpeed = 20.0f;
		bool _onGround = false;
		bool _firstPerson = true;

		int _activeAbility;
		std::vector<void(PlayerComponent::*)()> _abilityList;
		std::vector<int> _cooldownList;

		float _debug;
		glm::vec3 _debugPos;


	};
};
