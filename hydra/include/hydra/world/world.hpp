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

namespace Hydra::Renderer { struct HYDRA_API DrawObject; }
namespace Hydra::Physics { struct HYDRA_API PhysicsObject; }

namespace Hydra::Component {
	struct HYDRA_API TransformComponent;
	struct HYDRA_API CameraComponent;
	struct HYDRA_API LightComponent;
	struct HYDRA_API MeshComponent;
	struct HYDRA_API ParticleComponent;
	struct HYDRA_API EnemyComponent;
	struct HYDRA_API BulletComponent;
	struct HYDRA_API PlayerComponent;
	struct HYDRA_API WeaponComponent;
	struct HYDRA_API GrenadeComponent;
	struct HYDRA_API MineComponent;
	struct HYDRA_API RigidBodyComponent;
};

namespace Hydra::World {
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
	};
#undef BIT

	inline ComponentBits& operator |=(ComponentBits& a, const ComponentBits& b) { *reinterpret_cast<uint64_t*>(&a) |= static_cast<uint64_t>(b); return a;}
	inline ComponentBits& operator &=(ComponentBits& a, const ComponentBits& b) { *reinterpret_cast<uint64_t*>(&a) &= static_cast<uint64_t>(b); return a;}
	inline ComponentBits  operator | (const ComponentBits a, const ComponentBits b) { return static_cast<ComponentBits>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); }
	inline ComponentBits  operator & (const ComponentBits a, const ComponentBits b) { return static_cast<ComponentBits>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b)); }
	inline ComponentBits  operator ~ (const ComponentBits a) { return static_cast<ComponentBits>(~static_cast<uint64_t>(a)); }

	typedef size_t EntityID;
	template<typename T>
	constexpr T bitOr(T v) { return v; }

	template<typename T, typename... Args>
	constexpr T bitOr(T first, Args... args) { return first | combine(args...); }

	typedef void (*RemoveComponent_f)(EntityID id);
	const std::map<ComponentBits, RemoveComponent_f> removeComponent;

	struct HYDRA_API Entity final {
		// Entity Core
		EntityID id;
		ComponentBits activeComponents;
		EntityID parent;
		std::vector<EntityID> children;

		// Extra data
		std::string name;
		bool dead = true;

		~Entity();

		inline bool hasComponents(ComponentBits cb) { return (activeComponents & cb) == cb; }

		template <typename T>
		inline std::shared_ptr<T> getComponent() {
			if (hasComponents(T::bits))
				return T::getComponent(id);
			return std::shared_ptr<T>();
		}

		template <typename T>
		inline std::shared_ptr<T> addComponent() {
			if (hasComponents(T::bits))
				return T::getComponent(id);
			activeComponents |= T::bits;
			return T::addComponent(id);
		}

		template <typename T>
			inline void removeComponent() {
			if (!hasComponents(T::bits))
				return;
			activeComponents &= ~T::bits;
			T::removeComponent(id);
		}

		inline void addChild(EntityID entity) { children.push_back(entity); };

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
	};

	struct HYDRA_API IComponentBase {
		virtual ~IComponentBase() = 0;

		virtual const std::string type() const = 0;
		virtual void serialize(nlohmann::json& json) const = 0;
		virtual void deserialize(nlohmann::json& json) = 0;
		virtual void registerUI() = 0;
	};
	inline IComponentBase::~IComponentBase() {}

	template <typename T, ComponentBits bit>
	struct HYDRA_API IComponent : public IComponentBase {
		friend class Entity;
		static constexpr ComponentBits bits = bit;

		EntityID entityID;
		virtual ~IComponent() = 0;

		static inline const std::vector<std::shared_ptr<IComponent<T, bit>>>& getActiveComponents() { return _components; }

	private:
		static std::unordered_map<EntityID, size_t> _map;
		static std::vector<std::shared_ptr<IComponent<T, bit>>> _components;

		inline static std::shared_ptr<T> getComponent(EntityID entityID) {
			return std::static_pointer_cast<T>(_components[_map[entityID]]);
		}

		inline static std::shared_ptr<T> addComponent(EntityID entityID) {
			auto t = new T();
			t->entityID = entityID;
			_components.emplace_back(std::shared_ptr<IComponent<T, bit>>(t));
			_map[entityID] = _components.size() - 1;
			return std::static_pointer_cast<T>(_components.back());
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
	template <typename T, ComponentBits bit>
	inline IComponent<T, bit>::~IComponent() {}
	template <typename T, ComponentBits bit>
	std::unordered_map<EntityID, size_t> IComponent<T, bit>::_map;
	template <typename T, ComponentBits bit>
	std::vector<std::shared_ptr<IComponent<T, bit>>> IComponent<T, bit>::_components;

	struct HYDRA_API World final {
		static std::shared_ptr<Entity> root;

		World() = delete;

		static void reset() {
			_entities.clear();
			_map.clear();
			_idCounter = 1;
			root = newEntity("World Root", 0);
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
		inline static void getEntitiesWithComponents(std::vector<Entity*>& output) {
			output.clear();
			const ComponentBits bits = combine(Components::bits...);
			for (const Component0& c : Component0::getActiveComponents())
				if (auto e = getEntity(c.entityID); e->hasComponents(bits))
					output.push_back(e);
		}
	private:
		static std::unordered_map<EntityID, size_t> _map;
		static std::vector<std::shared_ptr<Entity>> _entities;
		static EntityID _idCounter;
	};

	class HYDRA_API ISystem {
	public:
		virtual ~ISystem() = 0;

		virtual void tick(World& world, float delta) = 0;

		virtual const std::string type() const = 0;
		virtual void registerUI() = 0;
	};
	inline ISystem::~ISystem() {}

	struct HYDRA_API Blueprint final {
		std::string name; // Blueprint Name

		inline nlohmann::json& getData() { return _root["data"]; }

		Entity* spawn(World& world);

		nlohmann::json _root;
	};
};
