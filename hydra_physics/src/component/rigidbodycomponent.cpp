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
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btMotionState.h>
#include <imgui/imgui.h>

using namespace Hydra::Component;
using namespace Hydra::World;
using namespace Hydra::Physics;

inline static btQuaternion cast(const glm::quat& r) { return btQuaternion{r.x, r.y, r.z, r.w}; }
inline static btVector3 cast(const glm::vec3& v) { return btVector3{v.x, v.y, v.z}; }

inline static glm::quat cast(const btQuaternion& r) { return glm::quat(r.w(), r.x(), r.y(), r.z()); }
inline static glm::vec3 cast(const btVector3& v) { return glm::vec3(v.x(), v.y(), v.z()); }

class ICallback {
public:
	typedef void (*Callback)(void* userptr);
	virtual void setCallback(Callback cb, void* userptr) = 0;
};

class MotionStateImpl final : public btMotionState, public ICallback {
public:
	MotionStateImpl(TransformComponent* transform) : _transform(transform) {}

	~MotionStateImpl() final {}

	void setCallback(Callback cb, void* userptr) final {
		_cb = cb;
		_userptr = userptr;
	}

	// This will only be called once, and only need to return the initial. No cache needed
	void getWorldTransform(btTransform& worldTransform) const { worldTransform = btTransform(cast(_transform->getRotation()), cast(_transform->getPosition())); }

	void setWorldTransform(const btTransform& worldTransform)	{
		_transform->setRotation(cast(worldTransform.getRotation()));
		_transform->setPosition(cast(worldTransform.getOrigin()));
		if (_cb)
			_cb(_userptr);
	}
private:
	TransformComponent* _transform;
	Callback _cb;
	void* _userptr;
};

typedef void (*SerializeShape)(nlohmann::json& json, btCollisionShape* shape);

static btCollisionShape* createShape(CollisionShape collisionShape, nlohmann::json& json) {
	(void)collisionShape;
	(void)json;
	return nullptr;
}

static SerializeShape getShapeSerializer(CollisionShape collisionShape) {
	(void)collisionShape;
	return nullptr;
}

struct RigidBodyComponent::Data {
	Data(CollisionShape collisionShape, SerializeShape serializeShape, TransformComponent* transform, btCollisionShape* shape, float mass, float linearDamping, float angularDamping, float friction, float rollingFriction) :
		collisionShape(collisionShape), serializeShape(serializeShape), motionState(transform), shape(shape), mass(mass), linearDamping(linearDamping), angularDamping(angularDamping), friction(friction), rollingFriction(rollingFriction),
		rigidBody([&]() {
				btRigidBody::btRigidBodyConstructionInfo info(mass, &motionState, shape);
				info.m_linearDamping = linearDamping;
				info.m_angularDamping = angularDamping;
				info.m_friction = friction;
				info.m_rollingFriction = rollingFriction;
				return info;
			}()) {}

	CollisionShape collisionShape;
	SerializeShape serializeShape;

	MotionStateImpl motionState;
	btCollisionShape* shape;

	float mass;
	float linearDamping;
	float angularDamping;
	float friction;
	float rollingFriction;

	btRigidBody rigidBody;
};

RigidBodyComponent::RigidBodyComponent(IEntity* entity) : IComponent(entity), _data(nullptr) {}
RigidBodyComponent::RigidBodyComponent(IEntity* entity, CollisionShape collisionShape, TransformComponent* transform, btCollisionShape* shape, float mass, float linearDamping, float angularDamping, float friction, float rollingFriction) : IComponent(entity), _data(std::make_unique<Data>(collisionShape, getShapeSerializer(collisionShape), transform, shape, mass, linearDamping, angularDamping, friction, rollingFriction)) {}

RigidBodyComponent::~RigidBodyComponent() {}
void RigidBodyComponent::tick(TickAction action, float delta) {(void)action; (void)delta;}
void RigidBodyComponent::serialize(nlohmann::json& json) const {
	json = {
		{"collisionShape", static_cast<size_t>(_data->collisionShape)},
		{"mass", _data->mass},
		{"linearDamping", _data->linearDamping},
		{"angularDamping", _data->angularDamping},
		{"friction", _data->friction},
		{"rollingFriction", _data->rollingFriction}
	};

	_data->serializeShape(json["shapeData"], _data->shape);
}

void RigidBodyComponent::deserialize(nlohmann::json& json) {
	_data = std::make_unique<Data>();
	auto collisionShape = static_cast<CollisionShape>(json["collisionShape"].get<size_t>());
	auto serializeShape = getShapeSerializer(collisionShape);
	auto shape = createShape(collisionShape, json["shapeData"]);

	auto mass = json["mass"].get<float>();
	auto linearDamping = json["linearDamping"].get<float>();
	auto angularDamping = json["angularDamping"].get<float>();
	auto friction = json["friction"].get<float>();
	auto rollingFriction = json["rollingFriction"].get<float>();

	_data(std::make_unique<Data>(collisionShape, serializeShape, entity->getComponent<Hydra::Component::TransformComponent>(), shape, mass, linearDamping, angularDamping, friction, rollingFriction))
}

void RigidBodyComponent::registerUI() {
	if (ImGui::DragFloat("Mass", &_data->mass))
		_data->rigidBody.setMassProps(_data->mass, cast(glm::vec3{0, 0, 0}));

	bool damping = ImGui::DragFloat("Linear Damping", &_data->linearDamping);
	damping |= ImGui::DragFloat("Angular Damping", &_data->angularDamping);
	if (damping)
		_data->rigidBody.setDamping(_data->linearDamping, _data->angularDamping);

	if (ImGui::DragFloat("Friction", &_data->friction))
		_data->rigidBody.setFriction(_data->friction);
	if (ImGui::DragFloat("Rolling Friction", &_data->rollingFriction))
		_data->rigidBody.setRollingFriction(_data->rollingFriction);

	//_data->shape->registerUI();
}
