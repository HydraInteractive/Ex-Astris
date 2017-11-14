#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <hydra/world/world.hpp>
class ComponentMenu
{
public:
	ComponentMenu();
	~ComponentMenu();
	void render(bool &openBool);
	void refresh();
	std::shared_ptr<Hydra::World::Entity> ComponentMenu::getRoomEntity();
	void configureComponent(bool &openBool, std::string componentType);
private:
	std::vector<std::weak_ptr<Hydra::World::Entity>> _entities;
	
	std::weak_ptr<Hydra::World::Entity> _selectedEntity = std::weak_ptr<Hydra::World::Entity>();
	std::vector<std::string> _componentTypes = {"Transform", "PointLight"};
	std::string _selectedString = "";
	void _menuBar();

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
		float constant = 1;
		float linear = 0.045f;
		float quadratic = 0.0075f;
	} pointLightInput;
};