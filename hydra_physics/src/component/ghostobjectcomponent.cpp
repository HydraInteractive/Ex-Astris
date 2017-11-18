#include <hydra/component/ghostobjectcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/drawobjectcomponent.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

//void Hydra::Component::GhostObjectComponent::createSphere(float radius){
//	ghostObject = new btGhostObject();
//	ghostObject->setCollisionShape(new btSphereShape(radius));
//	ghostObject->setWorldTransform(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)));
//}

void Hydra::Component::GhostObjectComponent::createBox(const glm::vec3 & halfExtents, const glm::quat& quatRotation) {
	this->halfExtents = halfExtents;
	this->quatRotation = quatRotation;
	ghostObject = new btGhostObject();
	ghostObject->setCollisionShape(new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z)));

	Hydra::World::World::getEntity(entityID)->addComponent<DrawObjectComponent>();
	auto tc = Hydra::World::World::getEntity(entityID)->addComponent<TransformComponent>();

	ghostObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(tc->position.x, tc->position.y, tc->position.z)));
}

GhostObjectComponent::~GhostObjectComponent() { }

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

	createBox(halfExtents,quatRotation);
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