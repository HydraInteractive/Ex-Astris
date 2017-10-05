#pragma once
#include <hydra/ext/api.hpp>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <hydra/world/world.hpp>
#include <hydra/component/cameracomponent.hpp>
using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API EditorCameraComponent final : public IComponent{
	public:
		EditorCameraComponent(IEntity* entity);
		~EditorCameraComponent() final;

		void tick(TickAction action, float delta) final;
		// If you want to add more than one TickAction, combine them with '|' (The bitwise or operator) 
		inline TickAction wantTick() const final { return TickAction::physics; }

		inline const std::string type() const final { return "EditorCameraComponent"; }
		const glm::vec3 getPosition() { return _position; };
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

	private:
		glm::vec3 _position = glm::vec3(0, 5, 0);
		glm::vec3 _direction = glm::vec3(0, -1, 0);
	};

};

