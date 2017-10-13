/**
 * Everything related to the world, as an interface.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <typeinfo>
#include <typeindex>
#undef min
#undef max
#include <json.hpp>
#include <hydra/ext/macros.hpp>

namespace Hydra::Renderer { struct HYDRA_BASE_API DrawObject; }
namespace Hydra::Physics { struct HYDRA_BASE_API PhysicsObject; }

namespace Hydra::Component {
	// Each component will be one entry in this list
#define BIT(x) (1 << x)
	enum class ComponentBits : uint64_t {
		Transform = BIT(0),
		Camera = BIT(1),
		Light = BIT(2),
		Mesh = BIT(3),
		Particle = BIT(4),
		Enemy = BIT(5),
		Bullet = BIT(6),
		Player = BIT(7),
		Weapon = BIT(8),
		Grenade = BIT(9),
		Mine = BIT(10),
		RigidBody = BIT(11),
		EditorCamera = BIT(12),
		DrawObject = BIT(13),
	};
#undef BIT

	inline ComponentBits& operator |=(ComponentBits& a, const ComponentBits& b) { *reinterpret_cast<uint64_t*>(&a) |= static_cast<uint64_t>(b); return a; }
	inline ComponentBits& operator &=(ComponentBits& a, const ComponentBits& b) { *reinterpret_cast<uint64_t*>(&a) &= static_cast<uint64_t>(b); return a; }
	inline ComponentBits  operator | (const ComponentBits a, const ComponentBits b) { return static_cast<ComponentBits>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); }
	inline ComponentBits  operator & (const ComponentBits a, const ComponentBits b) { return static_cast<ComponentBits>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b)); }
	inline ComponentBits  operator ~ (const ComponentBits a) { return static_cast<ComponentBits>(~static_cast<uint64_t>(a)); }
}

namespace Hydra::World {
	template <typename T, Hydra::Component::ComponentBits bit>
	struct HYDRA_BASE_API IComponent;
}

namespace Hydra::Component {
	struct HYDRA_BASE_API TransformComponent;
	struct HYDRA_GRAPHICS_API CameraComponent;
	struct HYDRA_GRAPHICS_API LightComponent;
	struct HYDRA_GRAPHICS_API MeshComponent;
	struct HYDRA_GRAPHICS_API ParticleComponent;
	struct HYDRA_PHYSICS_API EnemyComponent;
	struct HYDRA_PHYSICS_API BulletComponent;
	struct HYDRA_PHYSICS_API PlayerComponent;
	struct HYDRA_PHYSICS_API WeaponComponent;
	struct HYDRA_PHYSICS_API GrenadeComponent;
	struct HYDRA_PHYSICS_API MineComponent;
	struct HYDRA_GRAPHICS_API RigidBodyComponent;
	struct HYDRA_GRAPHICS_API EditorCameraComponent;
	struct HYDRA_GRAPHICS_API DrawObjectComponent;

	using ComponentTypes = Hydra::Ext::TypeTuple<
		Hydra::World::IComponent<TransformComponent, ComponentBits::Transform>,
		Hydra::World::IComponent<CameraComponent, ComponentBits::Camera>,
		Hydra::World::IComponent<LightComponent, ComponentBits::Light>,
		Hydra::World::IComponent<MeshComponent, ComponentBits::Mesh>,
		Hydra::World::IComponent<ParticleComponent, ComponentBits::Particle>,
		Hydra::World::IComponent<EnemyComponent, ComponentBits::Enemy>,
		Hydra::World::IComponent<BulletComponent, ComponentBits::Bullet>,
		Hydra::World::IComponent<PlayerComponent, ComponentBits::Player>,
		Hydra::World::IComponent<WeaponComponent, ComponentBits::Weapon>,
		Hydra::World::IComponent<GrenadeComponent, ComponentBits::Grenade>,
		Hydra::World::IComponent<MineComponent, ComponentBits::Mine>,
		Hydra::World::IComponent<RigidBodyComponent, ComponentBits::RigidBody>,
		Hydra::World::IComponent<EditorCameraComponent, ComponentBits::EditorCamera>,
		Hydra::World::IComponent<DrawObjectComponent, ComponentBits::DrawObject>
	>;
};

namespace Hydra::World {
	typedef size_t EntityID;

	template <typename T>
	constexpr T combine() { return T(); }
	template <typename T>
	constexpr T combine(T v) { return v; }

	template<typename T, typename... Args>
	constexpr T combine(T first, Args... args) { return first | combine<T>(args...); }

	struct HYDRA_BASE_API Entity final {
		// Entity Core
		EntityID id;
		Hydra::Component::ComponentBits activeComponents;
		EntityID parent;
		std::vector<EntityID> children;

		// Extra data
		std::string name;
		bool dead = false;

		~Entity();

		inline bool hasComponents(Hydra::Component::ComponentBits cb) const { return (activeComponents & cb) == cb; }

		template <typename T>
		inline bool hasComponent() const { return (activeComponents & T::bits) == T::bits; }

		inline size_t componentCount() {
			// Counts the number of bits set in activeComponents

			uint64_t i = static_cast<uint64_t>(activeComponents);
			i = i - ((i >> 1) & 0x5555555555555555UL);
			i = (i & 0x3333333333333333UL) + ((i >> 2) & 0x3333333333333333UL);
			return (int)((((i + (i >> 4)) & 0xF0F0F0F0F0F0F0FUL) * 0x101010101010101UL) >> 56);
		}

		template <typename T>
		inline std::shared_ptr<T> getComponent() const {
			if (hasComponents(T::bits))
				return std::static_pointer_cast<T>(T::getComponent(id));
			return std::shared_ptr<T>();
		}

		template <typename T>
		inline std::shared_ptr<T> addComponent() {
			if (hasComponents(T::bits))
				return std::static_pointer_cast<T>(T::getComponent(id));
			activeComponents |= T::bits;
			return std::static_pointer_cast<T>(T::addComponent(id));
		}

		template <typename T>
		inline void removeComponent() {
			if (!hasComponents(T::bits))
				return;
			activeComponents &= ~T::bits;
			T::removeComponent(id);
		}

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
	};

	struct HYDRA_BASE_API IComponentBase {
		virtual ~IComponentBase() = 0;

		virtual const std::string type() const = 0;
		virtual void serialize(nlohmann::json& json) const = 0;
		virtual void deserialize(nlohmann::json& json) = 0;
		virtual void registerUI() = 0;
	};
	inline IComponentBase::~IComponentBase() {}

	template <typename T, Hydra::Component::ComponentBits bit>
	struct HYDRA_BASE_API IComponent : public IComponentBase {
		friend struct Entity;
		static constexpr Hydra::Component::ComponentBits bits = bit;

		EntityID entityID;
		virtual ~IComponent() = 0;

		// This would work if everything would be in the same library
		//static inline const std::vector<std::shared_ptr<T>>& getActiveComponents() { return _components; }
		static inline const std::vector<std::shared_ptr<IComponent<T, bit>>>& getActiveComponents() { return _components; }

		static std::unordered_map<EntityID, size_t> _map;
		static std::vector<std::shared_ptr<IComponent<T, bit>>> _components;

		inline static std::shared_ptr<IComponent<T, bit>> getComponent(EntityID entityID) {
			return _components[_map[entityID]];
		}

		inline static std::shared_ptr<IComponent<T, bit>> addComponent(EntityID entityID) {
			auto t = new T();
			t->entityID = entityID;
			_components.emplace_back(std::shared_ptr<IComponent<T, bit>>(t));
			_map[entityID] = _components.size() - 1;
			return _components.back();
		}

		inline static void removeComponent(EntityID entityID) {
			const size_t pos = _map[entityID];
			if (pos != _components.size() - 1) {
				_map[_components.back()->entityID] = pos;
				std::swap(_components[pos], _components.back());
			}

			_components.pop_back();
			_map.erase(entityID);
		}
	};

	struct HYDRA_BASE_API World final {
		static std::shared_ptr<Entity> root;

		World() = delete;
		static constexpr EntityID invalidID = 0;

		static void reset() {
			_entities.clear();
			_map.clear();
			_idCounter = 1;
			root = newEntity("World Root", invalidID);
		}

		inline static std::shared_ptr<Entity> newEntity(const std::string& name, std::shared_ptr<Entity>& parent) {
			return newEntity(name, parent->id);
		}
		inline static std::shared_ptr<Entity> newEntity(const std::string& name, Entity* parent) {
			return newEntity(name, parent->id);
		}

		inline static std::shared_ptr<Entity> newEntity(const std::string& name, EntityID parent) {
			EntityID id = _idCounter++;
			std::shared_ptr<Entity> e = std::make_shared<Entity>();
			e->id = id;
			e->name = name;
			e->parent = parent;
			if (parent)
				getEntity(parent)->children.push_back(id);

			_entities.emplace_back(std::move(e));
			_map[id] = _entities.size() - 1;
			return _entities.back();
		}

		inline static void removeEntity(EntityID entityID) {
			if (!getEntity(entityID))
				return;
			const size_t pos = _map[entityID];
			if (pos != _entities.size() - 1) {
				_map[_entities.back()->id] = pos;
				std::swap(_entities[pos], _entities.back());
			}

			_entities.pop_back();
			_map.erase(entityID);
		}

		inline static std::shared_ptr<Entity> getEntity(EntityID id) {
			if (!_map.count(id))
				return std::shared_ptr<Entity>();
			return _entities[_map[id]];
		}

		template <typename Component0, typename... Components>
		inline static void getEntitiesWithComponents(std::vector<std::shared_ptr<Entity>>& output) {
			output.clear();
			const Hydra::Component::ComponentBits bits = combine<Hydra::Component::ComponentBits>(Components::bits...);
			for (auto& c : Component0::getActiveComponents())
				if (auto e = getEntity(c->entityID); e->hasComponents(bits))
					output.push_back(e);
		}

		static std::unordered_map<EntityID, size_t> _map;
		static std::vector<std::shared_ptr<Entity>> _entities;
		static EntityID _idCounter;
	};

	class HYDRA_BASE_API ISystem {
	public:
		virtual ~ISystem() = 0;

		virtual void tick(float delta) = 0;

		virtual const std::string type() const = 0;
		virtual void registerUI() = 0;
	};
	inline ISystem::~ISystem() {}

	struct HYDRA_BASE_API Blueprint final {
		std::string name; // Blueprint Name

		inline nlohmann::json& getData() { return _root["data"]; }

		void spawn(std::shared_ptr<Entity>& root);

		nlohmann::json _root;
	};
};
