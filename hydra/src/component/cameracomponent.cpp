#include <hydra/component/cameracomponent.hpp>

#include <typeinfo>
#include <typeindex>


using namespace Hydra::Component;

CameraComponent::CameraComponent(IEntity* entity, const glm::vec3& position) : IComponent(entity), _position(position) {}

CameraComponent::~CameraComponent() {}

void CameraComponent::tick(TickAction action) {
	if (action == TickAction::physics)
		_position += glm::vec3{0, 0, 0};
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
