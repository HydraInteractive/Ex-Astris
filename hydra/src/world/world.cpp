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

class WorldImpl : public IWorld {
public:
	WorldImpl() : _id(0) {
		_root = Entity::createEmpty(this, "World");
	}

	size_t getFreeID() final { return _id++; }

	std::shared_ptr<IEntity> createEntity(const std::string& name) final { return _root->createEntity(name); }
	void tick(TickAction action, float delta) final { _root->tick(action, delta); }
	void setWorldRoot(std::shared_ptr<IEntity> root) final { _root = root; }
	std::shared_ptr<IEntity> getWorldRoot() final { return _root; }
	std::map<std::type_index, std::vector<IEntity*>>& getActiveComponentMap() { return _activeComponents; }

private:
	std::shared_ptr<IEntity> _root;
	std::map<std::type_index, std::vector<IEntity*>> _activeComponents;
	size_t _id;
};

class EntityImpl : public IEntity {
public:
	EntityImpl(IWorld* world, const std::string& name = "", IEntity* parent = nullptr) : IEntity(world), _name(name), _parent(parent) {}
	EntityImpl(IWorld* world, nlohmann::json& json, IEntity* parent = nullptr) : IEntity(world), _parent(parent) {
		_name = json["name"].get<std::string>();

		{
			auto& components = json["components"];
			auto it = components.begin();
			auto& createMap = ComponentManager::createOrGetComponentMap();
			for (size_t i = 0; i < components.size(); i++, it++) {
				try {
					auto* c = createMap.at(it.key())(this);
					c->deserialize(it.value());
				} catch (const std::out_of_range&)	{
					Hydra::IEngine::getInstance()->log(Hydra::LogLevel::error, "Component type '%s' not found!", it.key().c_str());
				}
			}
		}

		{
			auto& children = json["children"];
			for (size_t i = 0; i < children.size(); i++)
				createEntity(children[i]);
		}
	}

	virtual ~EntityImpl() {
		if (_drawObject)
			_drawObject->refCounter--;
	}

	void tick(TickAction action, float delta) final {
		if (action == TickAction::checkDead) {
			auto it = std::remove_if(_children.begin(), _children.end(), [](const std::shared_ptr<IEntity>& e) { return e->isDead(); });
			_children.erase(it, _children.end());
		} else
			for (auto& component : _components)
				if ((action & component.second->wantTick()) == action)
					component.second->tick(action, delta);

		for (auto& child : _children)
			if ((action & child->wantTick()) == action)
				child->tick(action, delta);
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
		world->getActiveComponentMap()[id].push_back(this);
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
		entity->setParent(this);
		return entity;
	}
	std::shared_ptr<IEntity> createEntity(const std::string& name) final {
		return spawn(std::shared_ptr<IEntity>(new EntityImpl(world, name, this)));
	}
	std::shared_ptr<IEntity> createEntity(nlohmann::json& json) final {
		return spawn(std::shared_ptr<IEntity>(new EntityImpl(world, json, this)));
	}

	void setParent(IEntity* parent) final { _parent = parent; }
	IEntity* getParent() final { return _parent; }
	const std::vector<std::shared_ptr<IEntity>>& getChildren() final { return _children; }

	void serialize(nlohmann::json& json, bool serializeChildren) const final {
		json["name"] = _name;

		{
			auto& c = json["components"];
			for (auto& component : _components)
				component.second->serialize(c[component.second->type()]);
		}

		if (serializeChildren) {
			auto& children = json["children"];
			for (size_t i = 0; i < _children.size(); i++)
				_children[i]->serialize(children[i]);
		}
	}

	void deserialize(nlohmann::json& json) final {
		_name = json["name"].get<std::string>();

		{
			auto& components = json["components"];
			auto it = components.begin();
			auto& createMap = ComponentManager::createOrGetComponentMap();
			for (size_t i = 0; i < components.size(); i++, it++) {
				try {
					auto* c = createMap.at(it.key())(this);
					c->deserialize(components[i]);
				} catch (const std::out_of_range&)	{
					Hydra::IEngine::getInstance()->log(Hydra::LogLevel::error, "Component type '%s' not found!", it.key().c_str());
				}
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

std::shared_ptr<IEntity> Blueprint::spawn(IWorld* world) {
	return Entity::createFromBlueprint(world, getData());
}

std::unique_ptr<IWorld> World::create() {
	return std::unique_ptr<IWorld>(new WorldImpl());
}

std::shared_ptr<IEntity> Entity::createEmpty(IWorld* world, const std::string& name) {
	return std::shared_ptr<IEntity>(new EntityImpl(world, name, nullptr));
}

std::shared_ptr<IEntity> Entity::createFromBlueprint(IWorld* world, nlohmann::json& json) {
	return std::shared_ptr<IEntity>(new EntityImpl(world, json, nullptr));
}

