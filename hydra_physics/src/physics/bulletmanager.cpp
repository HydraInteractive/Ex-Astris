// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Interface to manage the Bullet3 physics library.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/physics/bulletmanager.hpp>

#include <hydra/component/transformcomponent.hpp>

struct BulletRigidBodyImpl final : public Hydra::Physics::IBulletRigidBody {
public:
	BulletRigidBodyImpl() {}

	btRigidBody* getRigidBody() final { return &_rigidBody; }
private:
	btRigidBody _rigidBody;
};

inline static btQuaternion cast(const glm::quat& r) { return btQuaternion{r.x, r.y, r.z, r.w}; }
inline static btVector3 cast(const glm::vec3& v) { return btVector3{v.x, v.y, v.z}; }

inline static glm::quat cast(const btQuaternion& r) { return glm::quat{r.x, r.y, r.z, r.w}; }
inline static glm::vec3 cast(const btVector3& v) { return glm::vec3{v.x, v.y, v.z}; }

struct BulletMotionStateImpl final : public Hydra::Physics::IBulletMotionState {
public:
	BulletMotionStateImpl(TransformComponent* transform) : _motionState(transform) {}

	btMotionState* getMotionState() final {	return &_motionState; }

	void setCallback(Callback cb, void* userptr) final { _motionState->setCallback(cb, userptr); }
private:
	class MotionStateImpl final : public btMotionState {
	public:
		MotionStateImpl(TransformComponent* transform) : _transform(transform) {}

    ~MotionStateImpl() final {}

		void setCallback(Callback cb, void* userptr) {
			_cb = cb;
			_userptr = userptr;
		}

		// This will only be called once, and only need to return the initial. No cache needed
    void getWorldTransform(btTransform& worldTransform) final const { worldTransform = btTransform(cast(transform->getRotation()), cast(transform->getPosition())); }

    void setWorldTransform(const btTransform& worldTransform) final	{
			_transform->setRotation(cast(worldTransform.getRotation()));
			_transform->setPosition(cast(worldTransform.getOrigin()));
			if (_cb)
				_cb(_userptr);
    }
	private:
		TransformComponent* _transform;
		Callback _cb;
		void* _userptr;
	};

	MotionStateImpl _motionState;
};

// FIXME: Make this less OOP, or try and introduct templates to remove alot of duplicated code
// Just need to fix a way for adding custom allocators while still begin a template or something.
struct BoxBulletShapeImpl : public Hydra::Physics::IBulletShape {
public:
	BoxBulletShapeImpl() _size({0, 0, 0}), _box(nullptr) {}
	BoxBulletShapeImpl(const glm::vec3& size) _size(size), _box(cast(size / 2)) {}

	inline btCollisionShape* getShape() final { return &_box; }
	inline CollisionShape getCollisionShape() final { return CollisionShape::box; }

	void serialize(nlohmann::json& json) const final {
		json = {
			{"size", {_size.x, _size.y, _size.z}}
		};
	}
	
	void deserialize(nlohmann::json& json) final {
		auto& s = json["size"];
		_size = glm::vec3{s[0].get<float>(), s[1].get<float>(), s[2].get<float>()};

		_box = btBoxShape(cast(_size / 2));
	}
	
	void registerUI() final {
		if (ImGui::DragFloat3("Size", glm::value_ptr(_size)))
			_box = btBoxShape(cast(_size / 2));
	}

private:
	glm::vec3 _size;
	btBoxShape _box;
};

class BulletManagerImpl final : public Hydra::Physics::IPhysicsManager {
public:
	BulletManagerImpl() {}
	~BulletManagerImpl() final {}

private:
	std::vector<Hydra::Physics::PhysicsObject> _physicsObjects;
};

std::unique_ptr<Hydra::Physics::IPhysicsManager> Hydra::Physics::BulletManager::create() {
	return std::unique_ptr<Hydra::Physics::IPhysicsManager>(new BulletManagerImpl());
}
