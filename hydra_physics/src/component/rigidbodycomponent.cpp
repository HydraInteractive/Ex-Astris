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
		_transform->setRotation(cast(worldTransform.getRotation()));
		_transform->setPosition(cast(worldTransform.getOrigin()));
		_transform->dirty = true;

		if (_cb)
			_cb(_userptr);
	}

	void setOffset(const glm::vec3 offset) {
		//_offset = cast(offset);
	}

	glm::vec3 getPosition() { return _transform->position; }
	glm::quat getRotation() { return _transform->rotation; }
private:
	std::shared_ptr<TransformComponent> _transform = nullptr;
	//btVector3 _offset = btVector3(0,0,0);
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
	case CollisionShape::CapsuleY:
		return std::unique_ptr<btCollisionShape>(new btCapsuleShape(json["radius"].get<float>(), json["height"].get<float>()));
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
			printf("halfExtents: %.2f, %.2f, %.2f\n", halfExtents.x(), halfExtents.y(), halfExtents.z());
			json["halfExtents"] = {halfExtents.x(), halfExtents.y(), halfExtents.z()};
		};
	case CollisionShape::StaticPlane:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btStaticPlaneShape* staticPlane = static_cast<btStaticPlaneShape*>(shape);

			auto n = staticPlane->getPlaneNormal();
			json["planeNormal"] = {n.x(), n.y(), n.z()};
			json["planeConstant"] = staticPlane->getPlaneConstant();
		};
	case CollisionShape::CapsuleY:
		return [](nlohmann::json& json, btCollisionShape* shape) {
			btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
			json["radius"] = capsule->getRadius();
			json["height"] = capsule->getHalfHeight() * 2;
		};
	default:
		assert(0);// Missing serializer
		return nullptr;
	}
}

struct RigidBodyComponent::Data {
	struct Shape {
		CollisionShape collisionShape;
		SerializeShape serializeShape;
		btTransform transform;
		std::unique_ptr<btCollisionShape> shape;
	};

	Data(EntityID entityID, Hydra::System::BulletPhysicsSystem::CollisionTypes collisionType, float mass, float linearDamping, float angularDamping, float friction, float rollingFriction) :
		motionState(entityID), entityID(entityID), mass(mass), linearDamping(linearDamping), angularDamping(angularDamping), friction(friction), rollingFriction(rollingFriction), collisionType(collisionType) { }

	btRigidBody* getRigidBody() {
		if (!rigidBody) {
			btRigidBody::btRigidBodyConstructionInfo info(mass, &motionState, &compoundShape);
			info.m_linearDamping = linearDamping;
			info.m_angularDamping = angularDamping;
			info.m_friction = friction;
			info.m_rollingFriction = rollingFriction;
			compoundShape.calculateLocalInertia(mass, info.m_localInertia);
			rigidBody = std::make_unique<btRigidBody>(info);
			rigidBody->setUserIndex(entityID);
			rigidBody->setUserIndex2(collisionType);
		}
		return rigidBody.get();
	}

	void addShape(Shape&& shape) {
		compoundShape.addChildShape(shape.transform, shape.shape.get());
		shapes.push_back(std::move(shape));
	}

	MotionStateImpl motionState;

	std::vector<Shape> shapes; // TODO: Share this among other RB
	btCompoundShape compoundShape; // New boi.

	// Add collision mask.
	EntityID entityID;
	float mass;
	float linearDamping;
	float angularDamping;
	float friction;
	float rollingFriction;
	Hydra::System::BulletPhysicsSystem::CollisionTypes collisionType;

	void serialize(nlohmann::json& json) {
		for (size_t i = 0; i < shapes.size(); i++) {
			auto& entry = json[i];
			auto& shape = shapes[i];
			entry["collisionShape"] = static_cast<size_t>(shape.collisionShape);
			shape.serializeShape(entry["data"], shape.shape.get());
			btTransformFloatData t;
			shape.transform.serializeFloat(t);

			entry["transformBasis"] = {
				{ t.m_basis.m_el[0].m_floats[0], t.m_basis.m_el[0].m_floats[1], t.m_basis.m_el[0].m_floats[2] },
				{ t.m_basis.m_el[1].m_floats[0], t.m_basis.m_el[1].m_floats[1], t.m_basis.m_el[1].m_floats[2] },
				{ t.m_basis.m_el[2].m_floats[0], t.m_basis.m_el[2].m_floats[1], t.m_basis.m_el[2].m_floats[2] }
			};
			entry["transformOrigin"] = { t.m_origin.m_floats[0], t.m_origin.m_floats[1], t.m_origin.m_floats[2] };
		}
	}

	void deserialize(size_t version, nlohmann::json& json) {
		if (version == 1) {
			btTransform localTrans;

			Shape shape;
			shape.collisionShape = static_cast<CollisionShape>(json["collisionShape"].get<size_t>());
			shape.serializeShape = getShapeSerializer(shape.collisionShape);
			shape.transform.setIdentity();
			shape.transform.setOrigin(cast(glm::vec3(0,0,0)));
			shape.shape = std::unique_ptr<btCollisionShape>(createShape(shape.collisionShape, json["shapeData"]));
			addShape(std::move(shape));
		} else {
			for (size_t i = 0; i < json.size(); i++) {
				auto& entry = json[i];

				auto& transformBasis = entry["transformBasis"];
				auto& transformOrigin = entry["transformOrigin"];

				btTransformFloatData t = {
					btMatrix3x3FloatData{
						btVector3FloatData{ transformBasis[0][0].get<float>(), transformBasis[0][1].get<float>(), transformBasis[0][2].get<float>() },
						btVector3FloatData{ transformBasis[1][0].get<float>(), transformBasis[1][1].get<float>(), transformBasis[1][2].get<float>() },
						btVector3FloatData{ transformBasis[2][0].get<float>(), transformBasis[2][1].get<float>(), transformBasis[2][2].get<float>() }
					},
					btVector3FloatData{ transformOrigin[0].get<float>(), transformOrigin[1].get<float>(), transformOrigin[2].get<float>() }
				};

				Shape shape;
				shape.collisionShape = static_cast<CollisionShape>(entry["collisionShape"].get<size_t>());
				shape.serializeShape = getShapeSerializer(shape.collisionShape);
				shape.transform.deSerializeFloat(t);
				shape.shape = std::unique_ptr<btCollisionShape>(createShape(shape.collisionShape, entry["data"]));
				addShape(std::move(shape));
			}
		}
	}

private:
	std::unique_ptr<btRigidBody> rigidBody;
};

RigidBodyComponent::~RigidBodyComponent() {
	if (_handler)
		_handler->disable(this);
	if (_data)
		delete _data;
}

#define DEFAULT_PARAMS Hydra::System::BulletPhysicsSystem::CollisionTypes collType, float mass, float linearDamping, float angularDamping, float friction, float rollingFriction
#define MAKE_DATA() _data = new Data(entityID, collType, mass, linearDamping, angularDamping, friction, rollingFriction)

void RigidBodyComponent::createBox(const glm::vec3& halfExtents, const glm::vec3& offset, DEFAULT_PARAMS) {
	btBoxShape* boxShape = new btBoxShape(cast(halfExtents));
	boxShape->setUserIndex(collType);

	if (!_data)
		MAKE_DATA();

	Data::Shape shape;
	shape.collisionShape = Hydra::System::BulletPhysicsSystem::CollisionShape::Box;
	shape.serializeShape = getShapeSerializer(shape.collisionShape);
	shape.transform.setIdentity();
	shape.transform.setOrigin(cast(offset));
	shape.shape = std::unique_ptr<btCollisionShape>(boxShape);
	_data->addShape(std::move(shape));
}

void RigidBodyComponent::createStaticPlane(const glm::vec3& planeNormal, float planeConstant, DEFAULT_PARAMS) {
	btStaticPlaneShape* plane = new btStaticPlaneShape(cast(planeNormal), planeConstant);
	plane->setUserIndex(collType);

	if (!_data)
		MAKE_DATA();

	Data::Shape shape;
	shape.collisionShape = Hydra::System::BulletPhysicsSystem::CollisionShape::StaticPlane;
	shape.serializeShape = getShapeSerializer(shape.collisionShape);
	shape.transform.setIdentity();
	shape.transform.setOrigin(btVector3(0, 0, 0));
	shape.shape = std::unique_ptr<btCollisionShape>(plane);
	_data->addShape(std::move(shape));
}

void RigidBodyComponent::createCapsuleY(float radius, float height, const glm::vec3& offset, DEFAULT_PARAMS) {
	btCapsuleShape* capsuleShape = new btCapsuleShape(radius, height);
	capsuleShape->setUserIndex(collType);

	if (!_data)
		MAKE_DATA();

	Data::Shape shape;
	shape.collisionShape = Hydra::System::BulletPhysicsSystem::CollisionShape::CapsuleY;
	shape.serializeShape = getShapeSerializer(shape.collisionShape);
	shape.transform.setIdentity();
	shape.transform.setOrigin(cast(offset));
	shape.shape = std::unique_ptr<btCollisionShape>(capsuleShape);
	_data->addShape( std::move(shape));
}

#undef DEFAULT_PARAMS

void RigidBodyComponent::setActivationState(ActivationState newState) {
	int lookup[static_cast<int>(ActivationState::MAX_COUNT)] = {ACTIVE_TAG, ISLAND_SLEEPING, WANTS_DEACTIVATION, DISABLE_DEACTIVATION, DISABLE_SIMULATION};
	_data->getRigidBody()->setActivationState(lookup[static_cast<int>(newState)]);
}

void RigidBodyComponent::refreshTransform() {
	btTransform t;
	_data->motionState.getWorldTransform(t);
	_data->getRigidBody()->setWorldTransform(t);
}

glm::vec3 RigidBodyComponent::getPosition(int childIndex) {
	return _data->motionState.getPosition() + cast(_data->compoundShape.getChildTransform(childIndex).getOrigin());
}

glm::quat RigidBodyComponent::getRotation() {
	return cast(_data->getRigidBody()->getWorldTransform().getRotation());
}

void Hydra::Component::RigidBodyComponent::setAngularForce(glm::vec3 angularForce){
	_data->getRigidBody()->setAngularFactor(btVector3(angularForce.x,angularForce.y,angularForce.z));
}

std::string RigidBodyComponent::getShapeString(int childIndex) {
	return ShapeTypesStr[_data->compoundShape.getChildShape(childIndex)->getShapeType()];
}

int RigidBodyComponent::getNumberOfChildren() {
	return _data->compoundShape.getNumChildShapes();
}

void* RigidBodyComponent::getRigidBody() { return static_cast<void*>(_data->getRigidBody()); }

glm::vec3 RigidBodyComponent::getHalfExtentScale() {
	for (Data::Shape& shape : _data->shapes)
		if (shape.collisionShape == Hydra::System::BulletPhysicsSystem::CollisionShape::Box)
			return cast(static_cast<btBoxShape*>(shape.shape.get())->getHalfExtentsWithMargin());
	return glm::vec3{0.5, 0.5, 0.5};
}

void RigidBodyComponent::serialize(nlohmann::json& json) const {
	json = {
		{"version", 2},
		{"mass", _data->mass},
		{"linearDamping", _data->linearDamping},
		{"angularDamping", _data->angularDamping},
		{"friction", _data->friction},
		{"rollingFriction", _data->rollingFriction},
		{"collisionType", static_cast<size_t>(_data->collisionType)}
	};

	_data->serialize(json["shapeData"]);
}

void RigidBodyComponent::deserialize(nlohmann::json& json) {
	auto version = json.value<size_t>("version", 1);
	auto mass = json["mass"].get<float>();
	auto linearDamping = json["linearDamping"].get<float>();
	auto angularDamping = json["angularDamping"].get<float>();
	auto friction = json["friction"].get<float>();
	auto rollingFriction = json["rollingFriction"].get<float>();
	auto collType = static_cast<Hydra::System::BulletPhysicsSystem::CollisionTypes>(json["collisionType"].get<int>());

	MAKE_DATA();
	_data->deserialize(version, version == 1 ? json : json["shapeData"]);
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

	if (ImGui::Button("Refresh from Transform"))
		refreshTransform();

	//_data->shape->registerUI();
}

#undef MAKE_DATA
