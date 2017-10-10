#pragma once
#include <hydra/ext/api.hpp>
#include <hydra/world/world.hpp>
#include <hydra/renderer/renderer.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>

#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
using namespace Hydra::World;

namespace Hydra::Component {
	class HYDRA_API EditorCameraComponent final : public IComponent{
	public:
		EditorCameraComponent(IEntity* entity);
		EditorCameraComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position = { 0, 0, 0 });
		~EditorCameraComponent() final;

		void tick(TickAction action, float delta) final;
		// If you want to add more than one TickAction, combine them with '|' (The bitwise or operator) 
		inline TickAction wantTick() const final { return TickAction::render; }

		inline const std::string type() const final { return "EditorCameraComponent"; }
		
		void serialize(nlohmann::json& json) const final;
		void deserialize(nlohmann::json& json) final;
		void registerUI() final;

		inline Hydra::Renderer::IRenderTarget* getRenderTarget() { return _renderTarget; }
		inline void setRenderTarget(Hydra::Renderer::IRenderTarget* renderTarget) { _renderTarget = renderTarget; }

		inline const glm::vec3& getPosition() const { return _position; }

		void setPosition(const glm::vec3& position);

		inline const float& getYaw() const { return _cameraYaw; }
		inline const float& getPitch() const { return _cameraPitch; }

		// TODO: Cache these?
		inline glm::mat4 getViewMatrix() const { return glm::translate(glm::mat4_cast(_orientation), -_position); }
		inline glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(_fov), (_renderTarget->getSize().x*1.0f) / _renderTarget->getSize().y, _zNear, _zFar); }

	private:
		Hydra::Renderer::IRenderTarget* _renderTarget;

		glm::vec3 _position = glm::vec3(0, 5, 0);
		int keysArrayLength;
		bool *lastKeysArray;
		float _movementSpeed = 10.0f;
		float _shiftMultiplier = 5.0f;

		glm::quat _orientation;

		float _fov = 90.0f;
		float _zNear = 0.001f;
		float _zFar = 75.0f;
		float _aspect = 16.0f/9.0f;

		float _sensitivity = 0.003f;
		float _cameraYaw = 0.0f;
		float _cameraPitch = 0.0f;


	};

};

