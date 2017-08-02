#include <hydra/world/world.hpp>

#include <algorithm>
#include <typeinfo>

using namespace Hydra::World;

// Is is 'virtual public' because WorldImpl needs it like that
class EntityImpl : virtual public IEntity {
public:
	EntityImpl() : _name("") {}
	EntityImpl(const std::string& name, IEntity* parent) : _name(name), _parent(parent) {}

	virtual ~EntityImpl() {}

	void tick(TickAction action) final {
		if (action == TickAction::checkDead) {
			auto it = std::remove_if(_children.begin(), _children.end(), [](const std::shared_ptr<IEntity>& e) { return e->isDead(); });
			_children.erase(it, _children.end());
		} else
			for (auto& component : _components)
				component->tick(action);

		for (auto& child : _children)
			child->tick(action);
	}

	void markDead() final { _dead = true; }

	void add(std::unique_ptr<IComponent> component) final { _components.push_back(std::move(component)); }
	void remove(IComponent* component) final {
		auto it = std::remove_if(_components.begin(), _components.end(), [&component](const std::unique_ptr<IComponent>& c) { return c.get() == component; });
		_components.erase(it, _components.end());
	}
	const std::vector<std::unique_ptr<IComponent>>& getComponents() final { return _components; }

	std::shared_ptr<IEntity> spawn(std::shared_ptr<IEntity> entity) final {
		_children.push_back(entity);
		return entity;
	}
	std::shared_ptr<IEntity> spawn(Blueprint& blueprint) final {
		// TODO:
		std::shared_ptr<IEntity> entity;
		_children.push_back(entity = std::shared_ptr<IEntity>(new EntityImpl(blueprint.name, this)));
		return entity;
	}

	IEntity* getParent() final { return _parent; }
	const std::vector<std::shared_ptr<IEntity>>& getChildren() final { return _children; }

	msgpack::packer<msgpack::sbuffer>& pack(msgpack::packer<msgpack::sbuffer>& o) const final {
		o.pack_map(3);

		o.pack("Name");
		o.pack(_name);

		o.pack("Components");
		o.pack_map(_components.size());
		for (auto& component : _components) {
			o.pack(component->type());
			component->pack(o);
		}

		o.pack("Children");
		o.pack_array(_children.size());
		for (auto& child : _children)
			child->pack(o);

		return o;
	}

	const std::string& getName() const final { return _name; }
	bool isDead() const final { return _dead; }

 protected:
	std::string _name;
	bool _dead = false;
	IEntity* _parent = nullptr;
	std::vector<std::unique_ptr<IComponent>> _components;
	std::vector<std::shared_ptr<IEntity>> _children;
};

// WorldImpl implement IWorld, and IWorld implements IEntity
// EntityImpl defines the code for all the IEntity functions, that is why it is like it is.
class WorldImpl final : virtual public IWorld, public EntityImpl {
public:
	WorldImpl() : EntityImpl("World", nullptr) {}
	~WorldImpl() final {}

	std::shared_ptr<IEntity> createEntity(const std::string& name) final {
		auto entity = std::shared_ptr<IEntity>(new EntityImpl(name, this));
		_children.push_back(entity);
		return entity;
	}
};

std::shared_ptr<IWorld> World::create() {
	return std::shared_ptr<IWorld>(new WorldImpl());
}
