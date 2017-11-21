#include <hydra/component/ghostobjectcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>
#include <glm/gtx/matrix_decompose.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

//void Hydra::Component::GhostObjectComponent::createSphere(float radius){
//	ghostObject = new btGhostObject();
//	ghostObject->setCollisionShape(new btSphereShape(radius));
//	ghostObject->setWorldTransform(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)));
//}

void Hydra::Component::GhostObjectComponent::_recalculateMatrix(){
	if (auto tc = Hydra::World::World::getEntity(entityID)->getComponent<TransformComponent>())
	{
		glm::vec3 newScale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(tc->getMatrix(), newScale, rotation, translation, skew, perspective);
		_matrix = glm::translate(translation) * glm::mat4_cast(quatRotation) * glm::scale(halfExtents);
	}
}

void Hydra::Component::GhostObjectComponent::createBox(const glm::vec3& halfExtents, Hydra::System::BulletPhysicsSystem::CollisionTypes collType, const glm::quat& quatRotation) {
	this->halfExtents = halfExtents;
	this->quatRotation = quatRotation;
	ghostObject = new btGhostObject();
	ghostObject->setCollisionShape(new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z)));
	ghostObject->setUserIndex(this->entityID);
	ghostObject->setUserIndex2(collType);
	ghostObject->setFriction(0);
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

	if (rotation == glm::vec3()) {
		rotation == glm::vec3(0.00000000000001f);
		glm::quat qPitch = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0, 0, 1));
		quatRotation = glm::normalize(qPitch * qYaw * qRoll);
	}

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
		glm::quat qPitch = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1, 0, 0));
		glm::quat qYaw = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0, 1, 0));
		glm::quat qRoll = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0, 0, 1));
		quatRotation = glm::normalize(qPitch * qYaw * qRoll);

		ghostObject->getWorldTransform().setRotation(btQuaternion(quatRotation.x, quatRotation.y, quatRotation.z, quatRotation.w));
	}
}