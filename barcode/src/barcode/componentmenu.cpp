#include <barcode/componentmenu.hpp>

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/playercomponent.hpp>
#include <hydra/component/weaponcomponent.hpp>
#include <hydra/component/particlecomponent.hpp>
#include <hydra/component/aicomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/lightcomponent.hpp>
#include <hydra/component/pointlightcomponent.hpp>
#include <hydra/component/movementcomponent.hpp>
#include <hydra/component/lifecomponent.hpp>
#include <hydra/component/ghostobjectcomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <hydra/component/textcomponent.hpp>
#include <glm/gtc/type_ptr.hpp>
using world = Hydra::World::World;
ComponentMenu::ComponentMenu()
{
	glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
}
  
ComponentMenu::~ComponentMenu()
{

}  

void ComponentMenu::render(bool &openBool, Hydra::System::BulletPhysicsSystem& physicsSystem)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_Once);
	ImGui::Begin("Add component", &openBool, ImGuiWindowFlags_MenuBar);
	_menuBar();
	
	ImGui::Columns(3, "Columns");
	ImGui::Text("Select entity");
	if(getRoomEntity() != nullptr)
		_renderEntity(getRoomEntity().get());

	ImGui::NextColumn();
	ImGui::Text("Select component type");
	if (_selectedEntity != nullptr)
	{
		for (size_t i = 0; i < _componentTypes.size(); i++)
		{
			if (ImGui::MenuItem(_componentTypes[i].c_str(), "", (_selectedString == _componentTypes[i])))
			{
				_selectedString = _componentTypes[i];
			}
		}
	}
	
	ImGui::NextColumn();
	ImGui::Text("Configure component");
	if (_selectedString != "" && _selectedEntity != nullptr)
	{
		configureComponent(openBool, _selectedString, physicsSystem);
	}
	ImGui::End();
}

void ComponentMenu::refresh()
{
	_selectedString = "";
	_selectedEntity = nullptr;
}

std::shared_ptr<Hydra::World::Entity> ComponentMenu::getRoomEntity()
{
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
	world::getEntitiesWithComponents<Hydra::Component::TransformComponent, Hydra::Component::RoomComponent>(entities);
	if (entities.size() > 0)
	{
		return entities[0];
	}
	return nullptr;
} 

void ComponentMenu::configureComponent(bool &openBool, std::string componentType, Hydra::System::BulletPhysicsSystem& physicsSystem)
{
	if (componentType == "Transform")
	{
		if (_selectedEntity->hasComponent<Hydra::Component::TransformComponent>())
		{
			ImGui::Text("The entity selected already has this component");
		}
		else 
		{
			ImGui::BeginChild("Transform", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, ImGui::GetWindowContentRegionMax().y - 160), true);
			ImGui::DragFloat3("Position", glm::value_ptr(transformInput.position));
			ImGui::DragFloat3("Scale", glm::value_ptr(transformInput.scale));
			ImGui::DragFloat4("Rotation", glm::value_ptr(transformInput.rotation));
			ImGui::Checkbox("Ignore parent", &transformInput.ignoreParent);
			ImGui::EndChild();
			ImGui::BeginChild("Confirm", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, 25));
			if (ImGui::Button("Finish"))
			{
				auto t = _selectedEntity->addComponent<Hydra::Component::TransformComponent>();
				t->position = transformInput.position;
				t->scale = transformInput.scale;
				t->rotation = transformInput.rotation;
				t->ignoreParent = transformInput.ignoreParent;
				transformInput = TI();
				//openBool = false;
			}
			ImGui::EndChild();
		}
	}
	else if (componentType == "PointLight")
	{
		if (_selectedEntity->hasComponent<Hydra::Component::PointLightComponent>())
		{
			ImGui::Text("The entity selected already has this component");
		}
		else
		{
			ImGui::BeginChild("Point light", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, ImGui::GetWindowContentRegionMax().y - 160), true);
			ImGui::DragFloat3("Colour", glm::value_ptr(pointLightInput.colour), 0.01f);
			ImGui::DragFloat("Constant", &pointLightInput.constant, 0.0001f);
			ImGui::DragFloat("Linear", &pointLightInput.linear, 0.01f);
			ImGui::DragFloat("Quadratic", &pointLightInput.quadratic, 0.0001f);
			ImGui::EndChild();
			ImGui::BeginChild("Confirm", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, 25));
			if (ImGui::Button("Finish"))
			{
				auto p = _selectedEntity->addComponent<Hydra::Component::PointLightComponent>();
				p->color = pointLightInput.colour;
				p->constant = pointLightInput.constant;
				p->linear = pointLightInput.linear;
				p->quadratic = pointLightInput.quadratic;
				pointLightInput = PLI();
				//openBool = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset"))
			{

			}
			ImGui::EndChild();
		}
	}

	else if (componentType == "RigidBody")
	{
		if (_selectedEntity->hasComponent<Hydra::Component::RigidBodyComponent>())
		{
			ImGui::Text("The entity selected already has this component");
		}
		else  
		{
			ImGui::BeginChild("RigidBody", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, ImGui::GetWindowContentRegionMax().y - 160), true);
			ImGui::DragFloat3("Size", glm::value_ptr(rigidBodyInput.size), 0.01f);
			ImGui::DragFloat("Mass", &rigidBodyInput.mass, 0.01f);
			ImGui::DragFloat("Linear Dampening", &rigidBodyInput.linearDampening, 0.01f);
			ImGui::DragFloat("Angular Dampening", &rigidBodyInput.angularDampening, 0.01f);
			ImGui::DragFloat("Friction", &rigidBodyInput.friction, 0.01f);
			ImGui::DragFloat("Rolling Friction", &rigidBodyInput.rollingFriction, 0.01f);

			//TODO: Selection box for picking collision type
			ImGui::EndChild();
			ImGui::BeginChild("Confirm", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, 25));
			if (ImGui::Button("Finish")) 
			{
				auto t = _selectedEntity->addComponent<Hydra::Component::RigidBodyComponent>();
				//physicsBox->addComponent<Hydra::Component::RigidBodyComponent>()->createBox(t->scale * 10.0f, Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_MISC_OBJECT, 10, 0, 0, 1.0f, 1.0f);
				t->createBox(rigidBodyInput.size/2.0f, glm::vec3(0), Hydra::System::BulletPhysicsSystem::CollisionTypes::COLL_MISC_OBJECT, rigidBodyInput.mass);
				physicsSystem.enable(t.get());
				rigidBodyInput = RBI();
				//openBool = false;
			}
			ImGui::EndChild();
		}
	}

	else if (componentType == "StaticObject") {
		if (_selectedEntity->hasComponent<Hydra::Component::GhostObjectComponent>())
			ImGui::Text("The entity selected already has this component");
		else {
			ImGui::BeginChild("GhostObject", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, ImGui::GetWindowContentRegionMax().y - 160), true);
			ImGui::DragFloat3("Size", glm::value_ptr(ghostObjectInput.size), 0.01f);

			if (ImGui::Combo("Collision Type", &ghostObjectInput.collisionType, "Nothing\0Wall\0Player\0Enemy\0Player Projectile\0Enemy Projectile\0Misc Object\0Pickup Object"))
				printf("%s\n", std::to_string(ghostObjectInput.collisionType).c_str());

			//TODO: Selection box for picking collision type
			ImGui::EndChild();
			ImGui::BeginChild("Confirm", ImVec2(ImGui::GetWindowContentRegionWidth() *0.3f, 25));
			if (ImGui::Button("Finish"))
			{
				_selectedEntity->addComponent<Hydra::Component::TransformComponent>();
				_selectedEntity->addComponent<Hydra::Component::DrawObjectComponent>();
				auto goc = _selectedEntity->addComponent<Hydra::Component::GhostObjectComponent>();
				goc->createBox(ghostObjectInput.size,Hydra::System::BulletPhysicsSystem::CollisionTypes(ghostObjectInput.collisionType+1));

				physicsSystem.enable(goc.get());
				rigidBodyInput = RBI();
			}
			ImGui::EndChild();
		}
		
	}
}

void ComponentMenu::_menuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Refresh", NULL))
			{
				refresh();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void ComponentMenu::_renderEntity(Hydra::World::Entity* entity)
{
	if (ImGui::IsItemClicked())
	{
		_selectedEntity = entity;
		_selectedString = "";
	}
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	auto entityIDs = entity->children;
	for (size_t i = 0; i < entityIDs.size(); i++)
	{
		auto child = world::getEntity(entityIDs[i]);
		if (ImGui::TreeNodeEx(child.get(), nodeFlags | ((_selectedEntity == child.get()) ? ImGuiTreeNodeFlags_Selected : 0), "%s", child->name.c_str()))
		{
			_renderEntity(child.get());
			ImGui::TreePop();
		}
	}
}
