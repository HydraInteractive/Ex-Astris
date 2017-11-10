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
#include <hydra/component/componentmanager.hpp>

#include <hydra/engine.hpp>

using namespace Hydra::Component;
using namespace Hydra::World;
using namespace Hydra::Physics;

inline static btQuaternion cast(const glm::quat& r) { return btQuaternion{r.x, r.y, r.z, r.w}; }
inline static btVector3 cast(const glm::vec3& v) { return btVector3{v.x, v.y, v.z}; }

inline static glm::quat cast(const btQuaternion& r) { return glm::quat(r.w(), r.x(), r.y(), r.z()); }
inline static glm::vec3 cast(const btVector3& v) { return glm::vec3(v.x(), v.y(), v.z()); }

using CollisionShape = Hydra::System::BulletPhysicsSystem::CollisionShape;

class ICallback {
public:
	typedef void (*Callback)(void* userptr);
	virtual void setCallback(Callback cb, void* userptr) = 0;
};

class MotionStateImpl final : public btMotionState, public ICallback {
public:
	MotionStateImpl(EntityID entityID) {
		_transform = Hydra::World::World::getEntity(entityID)->addComponent<TransformComponent>();
	}

	~MotionStateImpl() final {}

	void setCallback(Callback cb, void* userptr) final {
		_cb = cb;
		_userptr = userptr;
	}

	// This will only be called once, and only need to return the initial. No cache needed
	void getWorldTransform(btTransform& worldTransform) const {
		worldTransform = btTransform(cast(_transform->rotation), cast(_transform->position));
	}

	void setWorldTransform(const btTransform& worldTransform)	{
		_transform->rotation = cast(worldTransform.getRotation());
		_transform->position = cast(worldTransform.getOrigin());
		_transform->dirty = true;

		if (_cb)
			_cb(_userptr);
	}
private:
	std::shared_ptr<TransformComponent> _transform = nullptr;
	Callback _cb = nullptr;
	void* _userptr = nullptr;
};

typedef void (*SerializeShape)(nlohmann::json& json, btCollisionShape* shape);

static std::unique_ptr<btCollisionShape> createShape(CollisionShape collisionShape, nlohmann::json& json) {
	switch (collisionShape) {
	case CollisionShape::Box: {
		auto halfExtents = json["halfExtents"];
		return std::unique_ptr<btCollisionShape>(new btBoxShape(btVector3(halfExtents[0].get<float>(), halfExtents[1].get<float>(), halfExtents[2].get<float>())));
	}
	case CollisionShape::StaticPlane: {
		auto planeNormal = json["planeNormal"];
		auto planeConstant = json["planeConstant"];
		return std::unique_ptr<btCollisionShape>(new btStaticPlaneShape(btVector3(planeNormal[0].get<float>(), planeNormal[1].get<float>(), planeNormal[2].get<float>()), planeConstant.get<float>()));
	}
	case CollisionShape::Sphere:
		return std::unique_ptr<btCollisionShape>(new btSphereShape(json["radius"].get<float>()));

	case CollisionShape::CapsuleX:
		return std::unique_ptr<btCollisionShape>(new btCapsuleShapeX(json["radius"].get<float>(), json["height"].get<float>()));
	case CollisionShape::CapsuleY:
		return std::unique_ptr<btCollisionShape>(new btCapsuleShape(json["radius"].get<float>(), json["height"].get<float>()));
	case CollisionShape::CapsuleZ:
		return std::unique_ptr<btCollisionShape>(new btCapsuleShapeZ(json["radius"].get<float>(), json["height"].get<float>()));

	case CollisionShape::CylinderX: {
		auto halfExtents = json["halfExtents"];
		return std::unique_ptr<btCollisionShape>(new btCylinderShapeX(btVector3(halfExtents[0].get<float>(), halfExtents[1].get<float>(), halfExtents[2].get<float>())));
	}
	case CollisionShape::CylinderY: {
		auto halfExtents = json["halfExtents"];
		return std::unique_ptr<btCollisionShape>(new btCylinderShape(btVector3(halfExtents[0].get<float>(), halfExtents[1].get<float>(), halfExtents[2].get<float>())));
	}
	case CollisionShape::CylinderZ: {
		auto halfExtents = json["halfExtents"];
		return std::unique_ptr<btCollisionShape>(new btCylinderShapeZ(btVector3(halfExtents[0].get<float>(), halfExtents[1].get<float>(), halfExtents[2].get<float>())));
	}
	default:
		assert(0);// Missing serializer
		return nullptr;
	}
}

static SerializeShape getShapeSerializer(CollisionShape collisionShape) {
	switch (collisionShape) {
	case CollisionShape::Box:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btBoxShape* box = static_cast<btBoxShape*>(shape);

			auto halfExtents = box->getHalfExtentsWithMargin();
			json["halfExtents"] = {halfExtents.x(), halfExtents.y(), halfExtents.z()};
		};
	case CollisionShape::StaticPlane:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btStaticPlaneShape* staticPlane = static_cast<btStaticPlaneShape*>(shape);

			auto n = staticPlane->getPlaneNormal();
			json["planeNormal"] = {n.x(), n.y(), n.z()};
			json["planeConstant"] = staticPlane->getPlaneConstant();
		};
	case CollisionShape::Sphere:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btSphereShape* sphere = static_cast<btSphereShape*>(shape);
			json["radius"] = sphere->getRadius();
		};
	case CollisionShape::CapsuleX:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btCapsuleShapeX* capsule = static_cast<btCapsuleShapeX*>(shape);
			json["radius"] = capsule->getRadius();
			json["height"] = capsule->getHalfHeight() * 2;
		};
	case CollisionShape::CapsuleY:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
			json["radius"] = capsule->getRadius();
			json["height"] = capsule->getHalfHeight() * 2;
		};
	case CollisionShape::CapsuleZ:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btCapsuleShapeZ* capsule = static_cast<btCapsuleShapeZ*>(shape);
			json["radius"] = capsule->getRadius();
			json["height"] = capsule->getHalfHeight() * 2;
		};
	case CollisionShape::CylinderX:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btCylinderShapeX* cylinder = static_cast<btCylinderShapeX*>(shape);
			auto halfExtents = cylinder->getHalfExtentsWithMargin();
			json["halfExtents"] = {halfExtents.x(), halfExtents.y(), halfExtents.z()};
		};
	case CollisionShape::CylinderY:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btCylinderShape* cylinder = static_cast<btCylinderShape*>(shape);
			auto halfExtents = cylinder->getHalfExtentsWithMargin();
			json["halfExtents"] = {halfExtents.x(), halfExtents.y(), halfExtents.z()};
		};
	case CollisionShape::CylinderZ:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btCylinderShapeZ* cylinder = static_cast<btCylinderShapeZ*>(shape);
			auto halfExtents = cylinder->getHalfExtentsWithMargin();
			json["halfExtents"] = {halfExtents.x(), halfExtents.y(), halfExtents.z()};
		};
	default:
		assert(0);// Missing serializer
		return nullptr;
	}
}

struct RigidBodyComponent::Data {
	Data(EntityID entityID, CollisionShape collisionShape, SerializeShape serializeShape, std::unique_ptr<btCollisionShape> shape, Hydra::System::BulletPhysicsSystem::CollisionTypes collType, float mass, float linearDamping, float angularDamping, float friction, float rollingFriction) :
		collisionShape(collisionShape), serializeShape(serializeShape), motionState(entityID), shape(std::move(shape)), collisionType(collType), mass(mass), linearDamping(linearDamping), angularDamping(angularDamping), friction(friction), rollingFriction(rollingFriction), eID(entityID){}


	btRigidBody* getRigidBody() {
		if (!rigidBody) {
			btRigidBody::btRigidBodyConstructionInfo info(mass, &motionState, this->shape.get());
			info.m_linearDamping = linearDamping;
			info.m_angularDamping = angularDamping;
			info.m_friction = friction;
			info.m_rollingFriction = rollingFriction;
			shape->calculateLocalInertia(mass, info.m_localInertia);
			rigidBody = std::make_unique<btRigidBody>(info);
			rigidBody->setUserIndex(eID);
			rigidBody->setUserIndex2(collisionType);
		}
		return rigidBody.get();
	}

	CollisionShape collisionShape;
	SerializeShape serializeShape;
	MotionStateImpl motionState;
	std::unique_ptr<btCollisionShape> shape; // TODO: Share this among other RB

	// Add collision mask.
	float mass;
	float linearDamping;
	float angularDamping;
	float friction;
	float rollingFriction;
	Hydra::System::BulletPhysicsSystem::CollisionTypes collisionType;
	EntityID eID;

private:
	std::unique_ptr<btRigidBody> rigidBody;
};

RigidBodyComponent::~RigidBodyComponent() {
	if (_handler)
		_handler->disable(this);
	delete _data;
}

#define DEFAULT_PARAMS Hydra::System::BulletPhysicsSystem::CollisionTypes collType, float mass, float linearDamping, float angularDamping, float friction, float rollingFriction
#define MAKE_DATA(SHAPE_TYPE, SHAPE_PTR) _data = new Data(entityID, CollisionShape::SHAPE_TYPE, getShapeSerializer(CollisionShape::SHAPE_TYPE), std::unique_ptr<btCollisionShape>(SHAPE_PTR), collType, mass, linearDamping,angularDamping, friction, rollingFriction)

void RigidBodyComponent::createBox(const glm::vec3& halfExtents, DEFAULT_PARAMS) {
	_halfExtents = halfExtents;
	MAKE_DATA(Box, new btBoxShape(cast(halfExtents)));
}

void RigidBodyComponent::createStaticPlane(const glm::vec3& planeNormal, float planeConstant, DEFAULT_PARAMS) {
	MAKE_DATA(StaticPlane, new btStaticPlaneShape(cast(planeNormal), planeConstant));
}

void RigidBodyComponent::createSphere(float radius, DEFAULT_PARAMS) {
	MAKE_DATA(Sphere, new btSphereShape(radius));
}

void RigidBodyComponent::createCapsuleX(float radius, float height, DEFAULT_PARAMS) {
	MAKE_DATA(CapsuleX, new btCapsuleShapeX(radius, height));
}
void RigidBodyComponent::createCapsuleY(float radius, float height, DEFAULT_PARAMS) {
	MAKE_DATA(CapsuleY, new btCapsuleShape(radius, height));
}
void RigidBodyComponent::createCapsuleZ(float radius, float height, DEFAULT_PARAMS) {
	MAKE_DATA(CapsuleZ, new btCapsuleShapeZ(radius, height));
}

void RigidBodyComponent::createCylinderX(const glm::vec3& halfExtents, DEFAULT_PARAMS) {
	MAKE_DATA(CylinderX, new btCylinderShapeX(cast(halfExtents)));
}
void RigidBodyComponent::createCylinderY(const glm::vec3& halfExtents, DEFAULT_PARAMS) {
	MAKE_DATA(CylinderY, new btCylinderShape(cast(halfExtents)));
}
void RigidBodyComponent::createCylinderZ(const glm::vec3& halfExtents, DEFAULT_PARAMS) {
	MAKE_DATA(CylinderZ, new btCylinderShapeZ(cast(halfExtents)));
}

#undef MAKE_DATA
#undef DEFAULT_PARAMS

void RigidBodyComponent::setActivationState(const int newState) {
	_data->getRigidBody()->setActivationState(newState);
}

void* RigidBodyComponent::getRigidBody() { return static_cast<void*>(_data->getRigidBody()); }
void RigidBodyComponent::serialize(nlohmann::json& json) const {
	json = {
		{"collisionShape", static_cast<size_t>(_data->collisionShape)},
		{"collisionType", static_cast<int>(_data->collisionType)},
		{"mass", _data->mass},
		{"linearDamping", _data->linearDamping},
		{"angularDamping", _data->angularDamping},
		{"friction", _data->friction},
		{"rollingFriction", _data->rollingFriction}
	};

	_data->serializeShape(json["shapeData"], _data->shape.get());
}

void RigidBodyComponent::deserialize(nlohmann::json& json) {
	auto collisionShape = static_cast<CollisionShape>(json["collisionShape"].get<size_t>());
	auto serializeShape = getShapeSerializer(collisionShape);
	auto shape = createShape(collisionShape, json["shapeData"]);

	auto mass = json["mass"].get<float>();
	auto linearDamping = json["linearDamping"].get<float>();
	auto angularDamping = json["angularDamping"].get<float>();
	auto friction = json["friction"].get<float>();
	auto rollingFriction = json["rollingFriction"].get<float>();
	auto collisionType = static_cast<Hydra::System::BulletPhysicsSystem::CollisionTypes>(json["collisionType"].get<int>());

	_data = new Data(entityID, collisionShape, serializeShape, std::move(shape), collisionType, mass, linearDamping, angularDamping, friction, rollingFriction);
}

void RigidBodyComponent::registerUI() {
	if (ImGui::DragFloat("Mass", &_data->mass))
		_data->getRigidBody()->setMassProps(_data->mass, cast(glm::vec3{0, 0, 0}));

	bool damping = ImGui::DragFloat("Linear Damping", &_data->linearDamping);
	damping |= ImGui::DragFloat("Angular Damping", &_data->angularDamping);
	if (damping)
		_data->getRigidBody()->setDamping(_data->linearDamping, _data->angularDamping);

	if (ImGui::DragFloat("Friction", &_data->friction))
		_data->getRigidBody()->setFriction(_data->friction);
	if (ImGui::DragFloat("Rolling Friction", &_data->rollingFriction))
		_data->getRigidBody()->setRollingFriction(_data->rollingFriction);

	//_data->shape->registerUI();
}
