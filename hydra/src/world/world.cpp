// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Everything related to the world, as an interface.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/world/world.hpp>

#include <algorithm>
#include <hydra/renderer/renderer.hpp>
#include <hydra/engine.hpp>
#include <hydra/component/componentmanager.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

Entity::~Entity() {
	for (EntityID child : children)
		World::removeEntity(child);

	using namespace Hydra::Component;
	if (hasComponents(ComponentBits::Transform))
		IComponent<TransformComponent, ComponentBits::Transform>::removeComponent(id);
	if (hasComponents(ComponentBits::Camera))
		IComponent<CameraComponent, ComponentBits::Camera>::removeComponent(id);
	if (hasComponents(ComponentBits::Light))
		IComponent<LightComponent, ComponentBits::Light>::removeComponent(id);
	if (hasComponents(ComponentBits::Mesh))
		IComponent<MeshComponent, ComponentBits::Mesh>::removeComponent(id);
	if (hasComponents(ComponentBits::Particle))
		IComponent<ParticleComponent, ComponentBits::Particle>::removeComponent(id);
	if (hasComponents(ComponentBits::Enemy))
		IComponent<EnemyComponent, ComponentBits::Enemy>::removeComponent(id);
	if (hasComponents(ComponentBits::Bullet))
		IComponent<BulletComponent, ComponentBits::Bullet>::removeComponent(id);
	if (hasComponents(ComponentBits::Player))
		IComponent<PlayerComponent, ComponentBits::Player>::removeComponent(id);
	if (hasComponents(ComponentBits::Weapon))
		IComponent<WeaponComponent, ComponentBits::Weapon>::removeComponent(id);
	if (hasComponents(ComponentBits::Grenade))
		IComponent<GrenadeComponent, ComponentBits::Grenade>::removeComponent(id);
	if (hasComponents(ComponentBits::Mine))
		IComponent<MineComponent, ComponentBits::Mine>::removeComponent(id);
	if (hasComponents(ComponentBits::RigidBody))
		IComponent<RigidBodyComponent, ComponentBits::RigidBody>::removeComponent(id);
}

std::shared_ptr<Entity> World::root = nullptr;
std::unordered_map<EntityID, size_t> World::_map;
std::vector<std::shared_ptr<Entity>> World::_entities;
EntityID World::_idCounter = 1;

void Entity::serialize(nlohmann::json& json) const {
	json["name"] = name;

	{
		auto& c = json["components"];
		for (auto& component : _components)
			component.second->serialize(c[component.second->type()]);
	}

	{
		auto& children = json["children"];
		for (size_t i = 0; i < _children.size(); i++)
			_children[i]->serialize(children[i]);
	}
}

void Entity::deserialize(nlohmann::json& json) {
	name = json["name"].get<std::string>();

	{
		auto& components = json["components"];
		auto it = components.begin();
		auto& createMap = ComponentManager::createOrGetComponentMap();
		for (size_t i = 0; i < components.size(); i++, it++) {
			try {
				auto* c = createMap.at(it.key())(this); // TODO: Change to find?
				c->deserialize(components[i]);
			} catch (const std::out_of_range&)	{
				Hydra::IEngine::getInstance()->log(Hydra::LogLevel::error, "Component type '%s' not found!", it.key().c_str());
			}
		}
	}
}
