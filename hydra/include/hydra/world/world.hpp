#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <msgpack.hpp>

namespace Hydra::Renderer { struct DrawObject; }

namespace Hydra::World {
	class IComponent;
	struct Blueprint;

	enum class TickAction {
		checkDead = 0,
		physics,
		render,
		renderTransparent,
		network
	};

	class IEntity {
	public:
		virtual ~IEntity() = 0;

		virtual void tick(TickAction action) = 0;
		// TODO: Add a wantTick field, to now talk parts of the tree that don't want those actions

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
		T* getComponent() { return static_cast<T*>(getComponents()[std::type_index(typeid(T))].get()); }

		virtual std::shared_ptr<IEntity> spawn(std::shared_ptr<IEntity> entity) = 0;
		virtual std::shared_ptr<IEntity> spawn(Blueprint& blueprint) = 0;

		virtual IEntity* getParent() = 0;
		virtual const std::vector<std::shared_ptr<IEntity>>& getChildren() = 0;

		virtual msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const = 0;

		virtual const std::string& getName() const = 0;
		virtual Hydra::Renderer::DrawObject* getDrawObject() = 0;
		virtual bool isDead() const = 0;
	};
	inline IEntity::~IEntity() {}

	class IComponent {
	public:
		inline IComponent(IEntity* entity) : entity(entity) {}
		virtual ~IComponent() = 0;

		virtual void tick(TickAction action) = 0;
		// TODO: Add a wantTick field, to now talk parts of the tree that don't want those actions

		virtual const std::string type() = 0;

		virtual msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const = 0;

		// TODO?: IComponent* getParent();
		// TODO?: std::vector<IComponent*> getChildren();
	protected:
		IEntity* entity;
	};
	inline IComponent::~IComponent() {}

	struct Blueprint final {
		msgpack::object_handle objectHandle;

		std::string name;
		msgpack::object_map* data;
	};

	class IWorld : public virtual IEntity {
	public:
		virtual ~IWorld() = 0;

		virtual std::shared_ptr<IEntity> createEntity(const std::string& name) = 0;
	};
	inline IWorld::~IWorld() {}

	struct World final {
		static std::shared_ptr<IWorld> create();
	};
};
