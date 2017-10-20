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
		Life = BIT(11),
		RigidBody = BIT(12),
		EditorCamera = BIT(13),
		DrawObject = BIT(14),
		PointLight = BIT(15),
		Movement = BIT(16)
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
	struct HYDRA_GRAPHICS_API PointLightComponent;
	struct HYDRA_PHYSICS_API LifeComponent;
	struct HYDRA_PHYSICS_API MovementComponent;


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
		Hydra::World::IComponent<DrawObjectComponent, ComponentBits::DrawObject>,
		Hydra::World::IComponent<PointLightComponent, ComponentBits::PointLight>,
		Hydra::World::IComponent<LifeComponent, ComponentBits::Life>,
		Hydra::World::IComponent<MovementComponent, ComponentBits::Movement>
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
				return std::static_pointer_cast<T>(T::componentHandler->getComponent(id));
			return std::shared_ptr<T>();
		}

		template <typename T>
		inline std::shared_ptr<T> addComponent() {
			if (hasComponents(T::bits))
				return std::static_pointer_cast<T>(T::componentHandler->getComponent(id));
			activeComponents |= T::bits;
			return std::static_pointer_cast<T>(T::componentHandler->addComponent(id));
		}

		template <typename T>
		inline void removeComponent() {
			if (!hasComponents(T::bits))
				return;
			activeComponents &= ~T::bits;
			T::componentHandler->removeComponent(id);
		}

		void serialize(nlohmann::json& json) const;
		void deserialize(nlohmann::json& json);
	};

	struct HYDRA_BASE_API IComponentBase {
		friend struct Entity;

		EntityID entityID;

		virtual ~IComponentBase() = 0;

		virtual const std::string type() const = 0;
		virtual void serialize(nlohmann::json& json) const = 0;
		virtual void deserialize(nlohmann::json& json) = 0;
		virtual void registerUI() = 0;
	};
	inline IComponentBase::~IComponentBase() {}

	class HYDRA_BASE_API IComponentHandler {
	public:
		virtual const std::vector<std::shared_ptr<IComponentBase>>& getActiveComponents() = 0;

		//virtual std::unordered_map<EntityID, size_t> _map;
		//virtual std::vector<std::shared_ptr<IComponent<T, bit>>> _components;

		virtual std::shared_ptr<IComponentBase> getComponent(EntityID entityID) = 0;
		virtual std::shared_ptr<IComponentBase> addComponent(EntityID entityID) = 0;
		virtual void removeComponent(EntityID entityID) = 0;
	};

	template <typename T>
	class ComponentHandler : public IComponentHandler {
	public:
		std::unordered_map<EntityID, size_t> _map;
		std::vector<std::shared_ptr<IComponentBase>> _components;

		const std::vector<std::shared_ptr<IComponentBase>>& getActiveComponents() final { return _components; }
	
		std::shared_ptr<IComponentBase> getComponent(EntityID entityID) final {
			return _components[_map[entityID]];
		}

		std::shared_ptr<IComponentBase> addComponent(EntityID entityID) final {
			auto t = new T();
			t->entityID = entityID;
			_components.emplace_back(std::shared_ptr<IComponentBase>(t));
			_map[entityID] = _components.size() - 1;
			return _components.back();
		}

		void removeComponent(EntityID entityID) final {
			const size_t pos = _map[entityID];
			if (pos != _components.size() - 1) {
				_map[_components.back()->entityID] = pos;
				std::swap(_components[pos], _components.back());
			}

			_components.pop_back();
			_map.erase(entityID);
		}
	};

	template <typename T, Hydra::Component::ComponentBits bit>
	struct HYDRA_BASE_API IComponent : public IComponentBase {
		static IComponentHandler* componentHandler;

		friend struct Entity;
		static constexpr Hydra::Component::ComponentBits bits = bit;

		virtual ~IComponent() = 0;
	};
#if defined(HYDRA_BASE_EXPORTS) && !defined(__linux__)
	template <>
	IComponentHandler* IComponent<Hydra::Component::TransformComponent, Hydra::Component::ComponentBits::Transform>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::CameraComponent, Hydra::Component::ComponentBits::Camera>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::LightComponent, Hydra::Component::ComponentBits::Light>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::MeshComponent, Hydra::Component::ComponentBits::Mesh>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::ParticleComponent, Hydra::Component::ComponentBits::Particle>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::EnemyComponent, Hydra::Component::ComponentBits::Enemy>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::BulletComponent, Hydra::Component::ComponentBits::Bullet>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::PlayerComponent, Hydra::Component::ComponentBits::Player>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::WeaponComponent, Hydra::Component::ComponentBits::Weapon>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::GrenadeComponent, Hydra::Component::ComponentBits::Grenade>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::MineComponent, Hydra::Component::ComponentBits::Mine>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::RigidBodyComponent, Hydra::Component::ComponentBits::RigidBody>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::EditorCameraComponent, Hydra::Component::ComponentBits::EditorCamera>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::DrawObjectComponent, Hydra::Component::ComponentBits::DrawObject>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::PointLightComponent, Hydra::Component::ComponentBits::PointLight>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::LifeComponent, Hydra::Component::ComponentBits::Life>::componentHandler;
	template <>
	IComponentHandler* IComponent<Hydra::Component::MovementComponent, Hydra::Component::ComponentBits::Movement>::componentHandler;
#endif

	template HYDRA_BASE_API struct IComponent<Hydra::Component::TransformComponent, Hydra::Component::ComponentBits::Transform>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::CameraComponent, Hydra::Component::ComponentBits::Camera>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::LightComponent, Hydra::Component::ComponentBits::Light>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::MeshComponent, Hydra::Component::ComponentBits::Mesh>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::ParticleComponent, Hydra::Component::ComponentBits::Particle>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::EnemyComponent, Hydra::Component::ComponentBits::Enemy>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::BulletComponent, Hydra::Component::ComponentBits::Bullet>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::PlayerComponent, Hydra::Component::ComponentBits::Player>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::WeaponComponent, Hydra::Component::ComponentBits::Weapon>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::GrenadeComponent, Hydra::Component::ComponentBits::Grenade>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::MineComponent, Hydra::Component::ComponentBits::Mine>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::RigidBodyComponent, Hydra::Component::ComponentBits::RigidBody>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::EditorCameraComponent, Hydra::Component::ComponentBits::EditorCamera>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::DrawObjectComponent, Hydra::Component::ComponentBits::DrawObject>;
	template HYDRA_GRAPHICS_API struct IComponent<Hydra::Component::PointLightComponent, Hydra::Component::ComponentBits::PointLight>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::LifeComponent, Hydra::Component::ComponentBits::Life>;
	template HYDRA_PHYSICS_API struct IComponent<Hydra::Component::MovementComponent, Hydra::Component::ComponentBits::Movement>;

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
			for (auto& c : Component0::componentHandler->getActiveComponents())
				if (auto e = getEntity(c->entityID); e && e->hasComponents(bits))
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

	protected:
		std::vector<std::shared_ptr<Entity>> entities;
	};
	inline ISystem::~ISystem() { }

	struct HYDRA_BASE_API Blueprint final {
		std::string name; // Blueprint Name

		inline nlohmann::json& getData() { return _root["data"]; }

		void spawn(std::shared_ptr<Entity>& root);

		nlohmann::json _root;
	};
};
