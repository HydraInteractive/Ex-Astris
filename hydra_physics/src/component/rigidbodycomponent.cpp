// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * A component that enables the entity to be updated with physics.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/component/rigidbodycomponent.hpp>

#include <btBulletCollisionCommon.h>
#include <imgui/imgui.h>

using namespace Hydra::Component;
using namespace Hydra::World;

struct RigidBodyComponent::Data {
	btRigidBody* rigidBody;
	btMotionState* motionState;
	btCollisionShape* shape;

	float mass;
	float linearDamping;
	float angularDamping;
	float friction;
	float rollingFriction;
};

RigidBodyComponent::RigidBodyComponent(IEntity* entity) : IComponent(entity), _data(std::make_unique<RigidBodyComponent::Data>()) {}
RigidBodyComponent::RigidBodyComponent(IEntity* entity, float mass, float linearDamping, float angularDamping, float friction, float rollingFriction) : IComponent(entity), _data(std::make_unique<Data>()) {
	_data->mass = mass;
	_data->linearDamping = linearDamping;
	_data->angularDamping = angularDamping;
	_data->friction = friction;
	_data->rollingFriction = rollingFriction;
}

RigidBodyComponent::~RigidBodyComponent() {}
void RigidBodyComponent::tick(TickAction action, float delta) {}
void RigidBodyComponent::serialize(nlohmann::json& json) const {
	if (!_data)
		return;
	json = {
		{"mass", _data->mass},
		{"linearDamping", _data->linearDamping},
		{"angularDamping", _data->angularDamping},
		{"friction", _data->friction},
		{"rollingFriction", _data->rollingFriction},
		//		{"collisionShape", _data->shape->getCollistionShape()}
	};
	//	_data->shape->serialize(json["shape"]);
}

void RigidBodyComponent::deserialize(nlohmann::json& json) {
	// XXX: BORKEN;
	_data->mass = json["mass"].get<float>();
	_data->linearDamping = json["linearDamping"].get<float>();
	_data->angularDamping = json["angularDamping"].get<float>();
	_data->friction = json["friction"].get<float>();
	_data->rollingFriction = json["rollingFriction"].get<float>();

	//	_data->shape->deserialize(json["shape"]);
}

void RigidBodyComponent::registerUI() {
	if (ImGui::DragFloat("Mass", &_data->mass))
		{}//_data->getRigidBody()->setMassProps(cast(_data->mass), cast(glm::vec3{0, 0, 0}));

	bool damping = ImGui::DragFloat("Linear Damping", &_data->linearDamping);
	damping |= ImGui::DragFloat("Angular Damping", &_data->angularDamping);
	if (damping)
		{}//_data->getRigidBody()->setDamping(cast(_data->linearDamping), cast(_data->angularDamping));

	if (ImGui::DragFloat("Friction", &_data->friction))
		{}//_data->getRigidBody()->setFriction(_data->friction);
	if (ImGui::DragFloat("Rolling Friction", &_data->rollingFriction))
		{}//_data->getRigidBody()->setRollingFriction(_data->rollingFriction);

	//_data->shape->registerUI();
}
