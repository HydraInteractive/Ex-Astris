#pragma once
#include <hydra/ext/api.hpp>

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <msgpack.hpp>

namespace Hydra::Renderer { struct HYDRA_API DrawObject; }

namespace Hydra::World {
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

	class HYDRA_API IEntity {
	public:
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
		virtual std::shared_ptr<IEntity> spawn(Blueprint& blueprint) = 0;
		virtual std::shared_ptr<IEntity> createEntity(const std::string& name) = 0;

		virtual IEntity* getParent() = 0;
		virtual const std::vector<std::shared_ptr<IEntity>>& getChildren() = 0;

		virtual msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const = 0;

		virtual const std::string& getName() const = 0;
		virtual Hydra::Renderer::DrawObject* getDrawObject() = 0;
		virtual bool isDead() const = 0;
	};
	inline IEntity::~IEntity() {}

	class HYDRA_API IComponent {
	public:
		inline IComponent(IEntity* entity) : entity(entity) {}
		virtual ~IComponent() = 0;

		virtual void tick(TickAction action) = 0;
		virtual TickAction wantTick() const = 0;

		virtual const std::string type() const = 0;

		virtual msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const = 0;
		virtual void registerUI() = 0;

		// TODO?: IComponent* getParent();
		// TODO?: std::vector<IComponent*> getChildren();
	protected:
		IEntity* entity;
	};
	inline IComponent::~IComponent() {}

	struct HYDRA_API Blueprint final {
		msgpack::object_handle objectHandle;

		std::string name;
		msgpack::object_map* data;
	};

	class HYDRA_API IWorld : public virtual IEntity {
	public:
		virtual ~IWorld() = 0;
	};
	inline IWorld::~IWorld() {}

	struct HYDRA_API World final {
		static std::shared_ptr<IWorld> create();
	};
};
