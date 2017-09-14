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

namespace Hydra::World {
	class HYDRA_API IEntity;
	class HYDRA_API IComponent;
	struct HYDRA_API Blueprint;

	// The tick function only accepts one value, but the wantTick is a bitfield
	enum class HYDRA_API TickAction {
		checkDead = 1 << 0,
		physics = 1 << 1,
		render = 1 << 2,
		renderTransparent = 1 << 3,
		network = 1 << 4
	};
	inline TickAction operator| (TickAction a, TickAction b) { return static_cast<TickAction>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
	inline TickAction operator& (TickAction a, TickAction b) { return static_cast<TickAction>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }

	class HYDRA_API IWorld {
	public:
		virtual ~IWorld() = 0;

		virtual size_t getFreeID() = 0;

		// To emulate a IEntity, kinda
		virtual std::shared_ptr<IEntity> createEntity(const std::string& name) = 0;
		virtual void tick(TickAction action) = 0;

		virtual void setWorldRoot(std::shared_ptr<IEntity> root) = 0;
		virtual std::shared_ptr<IEntity> getWorldRoot() = 0;
	};
	inline IWorld::~IWorld() {}

	class HYDRA_API IEntity {
	public:
		inline IEntity(IWorld* world) : world(world), id(world->getFreeID()) {}
		virtual ~IEntity() = 0;

		virtual void tick(TickAction action) = 0;
		virtual TickAction wantTick() = 0;

		virtual void markDead() = 0;

		virtual IComponent* addComponent_(const std::type_index& id, std::unique_ptr<IComponent> component) = 0;
		virtual void removeComponent_(const std::type_index& id) = 0;
		virtual std::map<std::type_index, std::unique_ptr<IComponent>>& getComponents() = 0;

		template <typename T, typename... Args, typename std::enable_if<std::is_base_of<IComponent, T>::value>::type* = nullptr>
		T* addComponent(Args... args) {
			T* ptr = new T(this, args...);
			addComponent_(std::type_index(typeid(T)), std::unique_ptr<IComponent>(ptr));
			return ptr;
		}
		template <typename T, typename std::enable_if<std::is_base_of<IComponent, T>::value>::type* = nullptr>
		void removeComponent() { removeComponent_(std::type_index(typeid(T))); }
		template <typename T, typename std::enable_if<std::is_base_of<IComponent, T>::value>::type* = nullptr>
		T* getComponent() {
			try {
				return static_cast<T*>(getComponents().at(std::type_index(typeid(T))).get());
			} catch (std::out_of_range) {
				return nullptr;
			}
		}

		virtual std::shared_ptr<IEntity> spawn(std::shared_ptr<IEntity> entity) = 0;
		virtual std::shared_ptr<IEntity> createEntity(const std::string& name) = 0;
		virtual std::shared_ptr<IEntity> createEntity(nlohmann::json& json) = 0;

		inline size_t getID() { return id; }

		virtual void setParent(IEntity* parent) = 0;
		virtual IEntity* getParent() = 0;
		virtual const std::vector<std::shared_ptr<IEntity>>& getChildren() = 0;

		virtual void serialize(nlohmann::json& json, bool serializeChildren = true) const = 0;
		virtual void deserialize(nlohmann::json& json) = 0;

		virtual const std::string& getName() const = 0;
		virtual Hydra::Renderer::DrawObject* getDrawObject() = 0;
		virtual bool isDead() const = 0;
	protected:
		IWorld* world;
		size_t id;
	};
	inline IEntity::~IEntity() {}

	class HYDRA_API IComponent {
	public:
		inline IComponent(IEntity* entity) : entity(entity) {}
		virtual ~IComponent() = 0;

		virtual void tick(TickAction action) = 0;
		virtual TickAction wantTick() const = 0;

		virtual const std::string type() const = 0;

		virtual void serialize(nlohmann::json& json) const = 0;
		virtual void deserialize(nlohmann::json& json) = 0;
		virtual void registerUI() = 0;

		// TODO?: IComponent* getParent();
		// TODO?: std::vector<IComponent*> getChildren();
	protected:
		IEntity* entity;
	};
	inline IComponent::~IComponent() {}

	struct HYDRA_API Blueprint final {
		std::string name; // Blueprint Name

		inline nlohmann::json& getData() { return _root["data"]; }

		std::shared_ptr<IEntity> spawn(IWorld* world);

		nlohmann::json _root;
	};

	namespace World {
		HYDRA_API std::unique_ptr<IWorld> create();
	};

	namespace Entity {
		HYDRA_API std::shared_ptr<IEntity> createEmpty(IWorld* world, const std::string& name);
		HYDRA_API std::shared_ptr<IEntity> createFromBlueprint(IWorld* world, nlohmann::json& json);
	};
};
