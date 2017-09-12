// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * A camera component that generates view and projection matrices.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/component/cameracomponent.hpp>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Hydra::Component;

CameraComponent::CameraComponent(IEntity* entity, Hydra::Renderer::IRenderTarget* renderTarget, const glm::vec3& position) : IComponent(entity), _renderTarget(renderTarget), _position(position) {}

CameraComponent::~CameraComponent() {}

void CameraComponent::tick(TickAction action) {
	// assert(action == TickAction::physics); // Can only be this due to wantTick
	_position += glm::vec3{0, 0, 0};

	if (_mouseControl) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		
		int mouseX, mouseY;
		SDL_GetRelativeMouseState(&mouseX, &mouseY);

		_cameraYaw += mouseX * _sensitivity;;
		_cameraPitch -= mouseY *_sensitivity;

		glm::quat qPitch = glm::angleAxis(_cameraPitch, glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(_cameraYaw, glm::vec3(0, 1, 0));

		_orientation = qPitch * qYaw;
		_orientation = glm::normalize(_orientation);
	}

}

void CameraComponent::translate(const glm::vec3& transform) {
	_position += transform * _orientation;
}

void CameraComponent::rotation(float angle, const glm::vec3& axis) {
	_orientation *= glm::angleAxis(angle, axis * _orientation);
}

CameraComponent& CameraComponent::yaw(float angle) { rotation(angle, {0, 1, 0}); return *this; }
CameraComponent& CameraComponent::pitch(float angle) { rotation(angle, {1, 0, 0}); return *this; }
CameraComponent& CameraComponent::roll(float angle) { rotation(angle, {0, 0, 1}); return *this; }


void Hydra::Component::CameraComponent::setPosition(const glm::vec3 & position)
{
	auto model = entity->getComponent<Component::TransformComponent>();
	model->setPosition(position - glm::vec3(0,0,3));
	_position = position;
}

msgpack::packer<msgpack::sbuffer>& CameraComponent::pack(msgpack::packer<msgpack::sbuffer>& o) const {
	o.pack_map(6);

	o.pack("position");
	o.pack_array(3);
	o.pack_float(_position.x);
	o.pack_float(_position.y);
	o.pack_float(_position.z);

	o.pack("orientation");
	o.pack_array(4);
	o.pack_float(_orientation.x);
	o.pack_float(_orientation.y);
	o.pack_float(_orientation.z);
	o.pack_float(_orientation.w);

	o.pack("fov");
	o.pack_float(_fov);

	o.pack("zNear");
	o.pack_float(_zNear);

	o.pack("zFar");
	o.pack_float(_zFar);

	o.pack("aspect");
	o.pack_float(_aspect);

	return o;
}

void CameraComponent::registerUI() {
	//TODO: Change if dirty flag is added!
	ImGui::DragFloat3("Position", glm::value_ptr(_position), 0.01f);
	ImGui::DragFloat4("Orientation", glm::value_ptr(_orientation), 0.01f);
	ImGui::DragFloat("FOV", &_fov);
	ImGui::DragFloat("Z Near", &_zNear, 0.001f);
	ImGui::DragFloat("Z Far", &_zFar);
	float aspect = (_renderTarget->getSize().x*1.0f) / _renderTarget->getSize().y;
	ImGui::InputFloat("Aspect", &aspect, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);
	ImGui::Checkbox("Mouse Control", &_mouseControl);
}
