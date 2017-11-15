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
#include <hydra/component/roomcomponent.hpp>
#include <hydra/component/freecameracomponent.hpp>
#include <glm/gtc/type_ptr.hpp>
using world = Hydra::World::World;
ComponentMenu::ComponentMenu()
{
	_entities = std::vector<std::weak_ptr<Hydra::World::Entity>>();
}

ComponentMenu::~ComponentMenu()
{

}

void ComponentMenu::render(bool &openBool)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiSetCond_Once);
	ImGui::Begin("Add component", &openBool, ImGuiWindowFlags_MenuBar);
	_menuBar();
	ImGui::Columns(3, "Columns");
	ImGui::Text("Select entity");
	for (int i = 0; i < _entities.size(); i++)
	{
		if (_entities[i].expired())
		{
			_entities.erase(_entities.begin() + i);
		}
		else if (ImGui::MenuItem(_entities[i].lock()->name.c_str(), "", (_selectedEntity.lock() == _entities[i].lock())))
		{
			_selectedEntity = _entities[i];
			_selectedString = "";
		}
	}
	ImGui::NextColumn();
	ImGui::Text("Select component type");
	if (!_selectedEntity.expired())
	{
		for (int i = 0; i < _componentTypes.size(); i++)
		{
			if (ImGui::MenuItem(_componentTypes[i].c_str(), "", (_selectedString == _componentTypes[i])))
			{
				_selectedString = _componentTypes[i];
			}
		}
	}

	ImGui::NextColumn();
	ImGui::Text("Configure component");
	if (_selectedString != "" && !_selectedEntity.expired())
	{
		configureComponent(openBool, _selectedString);
	}
	ImGui::End();
}

void ComponentMenu::refresh()
{
	_entities.clear();
	auto& entityIDs = getRoomEntity()->children;
	for (int i = 0; i < entityIDs.size(); i++)
	{
		_entities.push_back(world::getEntity(entityIDs[i]));
	}
	_selectedEntity = std::weak_ptr<Hydra::World::Entity>();
	_selectedString = "";
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

void ComponentMenu::configureComponent(bool &openBool, std::string componentType)
{
	if (componentType == "Transform")
	{
		if (_selectedEntity.lock()->hasComponent<Hydra::Component::TransformComponent>())
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
				auto& t = _selectedEntity.lock()->addComponent<Hydra::Component::TransformComponent>();
				t->position = transformInput.position;
				t->scale = transformInput.scale;
				t->rotation = transformInput.rotation;
				t->ignoreParent = transformInput.ignoreParent;
				transformInput = TI();
				openBool = false;
			}
			ImGui::EndChild();
		}
	}
	else if (componentType == "PointLight")
	{
		if (_selectedEntity.lock()->hasComponent<Hydra::Component::PointLightComponent>())
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
				auto& p = _selectedEntity.lock()->addComponent<Hydra::Component::PointLightComponent>();
				p->color = pointLightInput.colour;
				p->constant = pointLightInput.constant;
				p->linear = pointLightInput.linear;
				p->quadratic = pointLightInput.quadratic;
				pointLightInput = PLI();
				openBool = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset"))
			{

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