#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <hydra/world/world.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
class ComponentMenu
{
public:
	ComponentMenu();
	~ComponentMenu();
	void render(bool &openBool, Hydra::System::BulletPhysicsSystem& physicsSystem);
	void refresh();
	static std::shared_ptr<Hydra::World::Entity> getRoomEntity();
	void configureComponent(bool &openBool, std::string componentType, Hydra::System::BulletPhysicsSystem& physicsSystem);
private:
	Hydra::World::Entity* _selectedEntity = nullptr;
	std::vector<std::string> _componentTypes = {"Transform", "PointLight", "RigidBody", "StaticObject"};
	std::string _selectedString = "";
	void _menuBar();
	void _renderEntity(Hydra::World::Entity*);
	//Transform
	struct TI
	{
		glm::vec3 position = glm::vec3{ 0, 0, 0 };
		glm::vec3 scale = glm::vec3{ 1, 1, 1 };
		glm::quat rotation = glm::quat();
		bool ignoreParent = false;
	} transformInput;
	
	//PointLight
	struct PLI
	{
		glm::vec3 colour = glm::vec3(1.0f, 1.0f, 1.0f);
		float constant = 1.0f;
		float linear = 0.045f;
		float quadratic = 0.0075f;
	} pointLightInput;

	//RigidBody
	struct RBI
	{
		glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
		float mass = 0.0f;
		float linearDampening = 0.0f;
		float angularDampening = 0.0f;
		float friction = 0.0f;
		float rollingFriction = 0.0f;
	} rigidBodyInput;
	
	//GhostObject
	struct GOI{
		glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
	} ghostObjectInput;
};