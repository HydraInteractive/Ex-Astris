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
#include <hydra/ext/macros.hpp>
#include <tuple>

using namespace Hydra::World;
using namespace Hydra::Component;

using ComponentTypes = Hydra::Ext::TypeTuple<
	IComponent<TransformComponent, ComponentBits::Transform>,
	IComponent<CameraComponent, ComponentBits::Camera>,
	IComponent<LightComponent, ComponentBits::Light>,
	IComponent<MeshComponent, ComponentBits::Mesh>,
	IComponent<ParticleComponent, ComponentBits::Particle>,
	IComponent<EnemyComponent, ComponentBits::Enemy>,
	IComponent<BulletComponent, ComponentBits::Bullet>,
	IComponent<PlayerComponent, ComponentBits::Player>,
	IComponent<WeaponComponent, ComponentBits::Weapon>,
	IComponent<GrenadeComponent, ComponentBits::Grenade>,
	IComponent<MineComponent, ComponentBits::Mine>,
	IComponent<RigidBodyComponent, ComponentBits::RigidBody>
>;


template <typename T>
inline void removeComponent(Entity& this_) {
	if (this_.hasComponent<T>())
		T::removeComponent(this_.id);
}

template <typename... Args>
struct RemoveComponents;

template <typename... Args>
struct RemoveComponents<Hydra::Ext::TypeTuple<Args...>> {
	constexpr static void apply(Entity& this_) {
		(removeComponent<Args>(this_), ...);
	}
};

Entity::~Entity() {
	for (EntityID child : children)
		World::removeEntity(child);
	RemoveComponents<ComponentTypes>::apply(*this);
}

std::shared_ptr<Entity> World::root = nullptr;
std::unordered_map<EntityID, size_t> World::_map;
std::vector<std::shared_ptr<Entity>> World::_entities;
EntityID World::_idCounter = 1;

template <typename T>
inline void serializeComponent(const Entity& this_, nlohmann::json& json) {
	if (this_.hasComponent<T>()) {
		auto component = this_.getComponent<T>();
		component->serialize(json[component->type()]);
	}
}

template <typename... Args>
struct SerializeComponents;

template <typename... Args>
struct SerializeComponents<Hydra::Ext::TypeTuple<Args...>> {
	constexpr static void apply(const Entity& this_, nlohmann::json& json) {
		(serializeComponent<Args>(this_, json), ...);
	}
};


void Entity::serialize(nlohmann::json& json) const {
	json["name"] = name;

	{
		auto& c = json["components"];
		SerializeComponents<ComponentTypes>::apply(*this, c);
	}

	{
		auto& c = json["children"];
		for (size_t i = 0; i < children.size(); i++)
			Hydra::World::World::getEntity(children[i])->serialize(c[i]);
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
