#include <hydra/component/ghostobjectcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <glm/gtx/matrix_decompose.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

inline static btQuaternion cast(const glm::quat& r) { return btQuaternion{ r.x, r.y, r.z, r.w }; }
inline static btVector3 cast(const glm::vec3& v) { return btVector3{ v.x, v.y, v.z }; }

inline static glm::quat cast(const btQuaternion& r) { return glm::quat(r.w(), r.x(), r.y(), r.z()); }
inline static glm::vec3 cast(const btVector3& v) { return glm::vec3(v.x(), v.y(), v.z()); }

void Hydra::Component::GhostObjectComponent::_recalculateMatrix(){
	auto tc = Hydra::World::World::getEntity(entityID)->addComponent<TransformComponent>();
	_matrix = tc->getMatrix() * glm::mat4_cast(quatRotation) * glm::scale(halfExtents);
}

void Hydra::Component::GhostObjectComponent::createBox(const glm::vec3& halfExtents, Hydra::System::BulletPhysicsSystem::CollisionTypes collType, const glm::quat& quatRotation) {
	this->halfExtents = halfExtents;
	this->quatRotation = quatRotation;
	
	ghostObject = new btGhostObject();
	ghostObject->setCollisionShape(new btBoxShape(cast(halfExtents)));
	ghostObject->setUserIndex(this->entityID);
	ghostObject->setUserIndex2(collType);
	ghostObject->setFriction(0);
}

void Hydra::Component::GhostObjectComponent::updateWorldTransform(){
	_recalculateMatrix();

	glm::vec3 newScale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(_matrix, newScale, rotation, translation, skew, perspective);

	delete ghostObject->getCollisionShape();
	ghostObject->setCollisionShape(new btBoxShape(cast(newScale)));
	ghostObject->setWorldTransform(btTransform(cast(rotation).inverse(), cast(_matrix[3])));
}

GhostObjectComponent::~GhostObjectComponent() {
	if (_handler)
		_handler->disable(this);
	delete ghostObject;
}

void GhostObjectComponent::serialize(nlohmann::json& json) const {
	json["halfExtentsX"] = halfExtents.x;
	json["halfExtentsY"] = halfExtents.y;
	json["halfExtentsZ"] = halfExtents.z;

	json["rotationX"] = rotation.x;
	json["rotationY"] = rotation.y;
	json["rotationZ"] = rotation.z;

	json["quatRotationX"] = quatRotation.x;
	json["quatRotationY"] = quatRotation.y;
	json["quatRotationZ"] = quatRotation.z;
	json["quatRotationW"] = quatRotation.w;

	json["collisionType"] = collisionType;
}

void GhostObjectComponent::deserialize(nlohmann::json& json) {
	halfExtents.x = json.value<float>("halfExtentsX", 0);
	halfExtents.y = json.value<float>("halfExtentsY", 0);
	halfExtents.z = json.value<float>("halfExtentsZ", 0);

	rotation.x = json.value<float>("rotationX", 0);
	rotation.y = json.value<float>("rotationY", 0);
	rotation.z = json.value<float>("rotationZ", 0);

	quatRotation.x = json.value<float>("quatRotationX", 0);
	quatRotation.y = json.value<float>("quatRotationY", 0);
	quatRotation.z = json.value<float>("quatRotationZ", 0);
	quatRotation.w = json.value<float>("quatRotationW", 0);

	collisionType = Hydra::System::BulletPhysicsSystem::CollisionTypes(json.value<int>("collisionType", 0));

	collisionType = Hydra::System::BulletPhysicsSystem::COLL_WALL;

	createBox(halfExtents, collisionType, quatRotation);
}

void GhostObjectComponent::registerUI() {
	if (ImGui::DragFloat3("Half Extents", glm::value_ptr(halfExtents), 0.01f)){
		delete ghostObject->getCollisionShape();
		ghostObject->setCollisionShape(new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z)));
	}
	if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 1.0f)){
		quatRotation = glm::quat(glm::radians(rotation));
		updateWorldTransform();
	}
	if (ImGui::Button("BOG")){
		updateWorldTransform();
	}
}