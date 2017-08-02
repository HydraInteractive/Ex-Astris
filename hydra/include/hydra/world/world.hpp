#pragma once

#include <string>
#include <memory>
#include <vector>
#include <msgpack.hpp>

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

		virtual void markDead() = 0;

		virtual void add(std::unique_ptr<IComponent> component) = 0;
		virtual void remove(IComponent* component) = 0;
		virtual const std::vector<std::unique_ptr<IComponent>>& getComponents() = 0;

		virtual std::shared_ptr<IEntity> spawn(std::shared_ptr<IEntity> entity) = 0;
		virtual std::shared_ptr<IEntity> spawn(Blueprint& blueprint) = 0;

		virtual IEntity* getParent() = 0;
		virtual const std::vector<std::shared_ptr<IEntity>>& getChildren() = 0;

		virtual msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const = 0;

		virtual const std::string& getName() const = 0;
		virtual bool isDead() const = 0;
	};
	inline IEntity::~IEntity() {}

	class IComponent {
	public:
		inline IComponent(std::weak_ptr<IEntity> entity) : entity(entity) {}
		virtual ~IComponent() = 0;

		virtual void tick(TickAction action) = 0;

		virtual const std::string type() = 0;

		virtual msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const = 0;

		// TODO?: IComponent* getParent();
		// TODO?: std::vector<IComponent*> getChildren();
	protected:
		std::weak_ptr<IEntity> entity;
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
