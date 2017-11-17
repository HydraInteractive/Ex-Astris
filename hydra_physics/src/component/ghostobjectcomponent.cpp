#include <hydra/component/ghostobjectcomponent.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/component/transformcomponent.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

void Hydra::Component::GhostObjectComponent::createSphere(float radius){
	ghostObject = new btGhostObject();
	ghostObject->setCollisionShape(new btSphereShape(radius));
	ghostObject->setWorldTransform(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)));
}

void Hydra::Component::GhostObjectComponent::createBox(const glm::vec3 & halfExtents){
	this->halfExtents = halfExtents;
	ghostObject = new btGhostObject();
	ghostObject->setCollisionShape(new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z)));

	auto tc = Hydra::World::World::getEntity(entityID)->addComponent<TransformComponent>();

	ghostObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(tc->position.x, tc->position.y, tc->position.z)));
}

GhostObjectComponent::~GhostObjectComponent() { }

void GhostObjectComponent::serialize(nlohmann::json& json) const {

}

void GhostObjectComponent::deserialize(nlohmann::json& json) {

}

void GhostObjectComponent::registerUI() {
	if (ImGui::DragFloat3("Half Extents", glm::value_ptr(halfExtents), 0.01f)) {
		delete ghostObject->getCollisionShape();
		ghostObject->setCollisionShape(new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z)));
	}
}
