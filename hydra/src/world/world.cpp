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

// Is is 'virtual public' because WorldImpl needs it like that
class EntityImpl : virtual public IEntity {
public:
	EntityImpl(size_t id) : IEntity(id),  _name("") {}
	EntityImpl(size_t id, const std::string& name, IEntity* parent) : IEntity(id), _name(name), _parent(parent) {}

	virtual ~EntityImpl() {
		if (_drawObject)
			_drawObject->refCounter--;
	}

	void tick(TickAction action) final {
		if (action == TickAction::checkDead) {
			auto it = std::remove_if(_children.begin(), _children.end(), [](const std::shared_ptr<IEntity>& e) { return e->isDead(); });
			_children.erase(it, _children.end());
		} else
			for (auto& component : _components)
				if ((action & component.second->wantTick()) == action)
					component.second->tick(action);

		for (auto& child : _children)
			if ((action & child->wantTick()) == action)
				child->tick(action);
	}

	TickAction wantTick() final {
		static TickAction want = TickAction::checkDead;

		if (!_wantDirty)
			return want;

		_wantDirty = false;

		for (auto& component : _components)
			want = want | component.second->wantTick();

		for (auto& child : _children)
			want = want | child->wantTick();

		return want;
	}

	void markDead() final { _dead = true; }

	IComponent* addComponent_(const std::type_index& id, std::unique_ptr<IComponent> component) final {
		_wantDirty = true;
		IComponent* ptr = component.get();
		_components[id] = std::move(component);
		return ptr;
	}

	void removeComponent_(const std::type_index& id) final {
		_wantDirty = true;
		_components.erase(id);
	}

	std::map<std::type_index, std::unique_ptr<IComponent>>& getComponents() final { return _components; }

	std::shared_ptr<IEntity> spawn(std::shared_ptr<IEntity> entity) final {
		_wantDirty = true;
		_children.push_back(entity);
		return entity;
	}
	std::shared_ptr<IEntity> spawn(Blueprint& blueprint) final {
		_wantDirty = true;
		// TODO:
		std::shared_ptr<IEntity> entity = createEntity(0, "");
		entity->deserialize(blueprint.json);
		return entity;
	}
	std::shared_ptr<IEntity> createEntity(size_t id, const std::string& name) final {
		return spawn(std::shared_ptr<IEntity>(new EntityImpl(id, name, this)));
	}

	IEntity* getParent() final { return _parent; }
	const std::vector<std::shared_ptr<IEntity>>& getChildren() final { return _children; }

	void serialize(nlohmann::json& json) const final {
		json["id"] = id;
		json["name"] = _name;

		{
			auto& c = json["components"];
			for (auto& component : _components)
				component.second->serialize(c[component.second->type()]);
		}

		{
			auto& children = json["children"];
			for (size_t i = 0; i < _children.size(); i++)
				_children[i]->serialize(children[std::to_string(_children[i]->getID())]);
		}
	}

	void deserialize(nlohmann::json& json) final {
		id = json["id"];
		_name = json["name"];

		{
			auto& components = json["components"];
			auto it = components.begin();
			for (size_t i = 0; i < components.size(); i++, it++) {
				try {
					auto* c = ComponentManager::createOrGetComponentMap().at(it.key())(this);
					c->deserialize(components[i]);
				} catch (const std::out_of_range&)	{
					Hydra::IEngine::getInstance()->log(Hydra::LogLevel::error, "Component type '%s' not found!", it.key().c_str());
				}
			}
		}

		{
			auto& children = json["children"];
			for (size_t i = 0; i < children.size(); i++) {
				size_t id = children[i]["id"].get<size_t>();
				auto childIt = std::find_if(_children.begin(), _children.end(), [id](std::shared_ptr<IEntity>& e) { return e->getID() == id; });
				std::shared_ptr<IEntity> child;
				if (childIt == _children.end())
					child = createEntity(0, "");
				else
					child = *childIt;
				child->deserialize(children[i]);
			}
		}
	}

	const std::string& getName() const final { return _name; }
	Hydra::Renderer::DrawObject* getDrawObject() final {
		if (!_drawObject) {
			_drawObject = Hydra::IEngine::getInstance()->getRenderer()->aquireDrawObject();
			_drawObject->refCounter++;
		}
		return _drawObject;
	}
	bool isDead() const final { return _dead; }

 protected:
	bool _wantDirty = false;
	std::string _name;
	bool _dead = false;
	IEntity* _parent = nullptr;
	std::map<std::type_index, std::unique_ptr<IComponent>> _components;
	std::vector<std::shared_ptr<IEntity>> _children;

	Hydra::Renderer::DrawObject* _drawObject = nullptr;
};

// WorldImpl implement IWorld, and IWorld implements IEntity
// EntityImpl defines the code for all the IEntity functions, that is why it is like it is.
class WorldImpl final : virtual public IWorld, public EntityImpl {
public:
	WorldImpl() : IEntity(0), EntityImpl(0, "World", nullptr) {}
	~WorldImpl() final {}
};

std::shared_ptr<IWorld> World::create() {
	return std::shared_ptr<IWorld>(new WorldImpl());
}

#undef DO_COMPONENTS
