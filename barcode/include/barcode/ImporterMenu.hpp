#pragma once
#include <string>
#include <vector>
#include <fstream>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
#include <imgui/icons.hpp>
#include <algorithm>
#include <hydra/world/world.hpp>
#include <hydra/world/blueprintloader.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>

class ImporterMenu
{
public:
	std::string executableDir;
	ImporterMenu();
	ImporterMenu(Hydra::World::IWorld* world);
	~ImporterMenu();

	void render(bool &closeBool);
	void refresh();
	static std::shared_ptr<IEntity> getRoomEntity(Hydra::World::IWorld* world);
private:

	class Node
	{
	public:
		bool isAllowedFile = false;

		Node();
		Node(std::string path, Node* parent = nullptr, bool isFile = false);
		~Node();

		std::string name();
		std::string getExt();
		std::string pathToName(std::string path);
		std::string reverseEngineerPath();
		int numberOfFiles();
		void clean();
		void render(Hydra::World::IWorld* world, Node** selectedNode);
	private:
		std::string _name;
		std::vector<Node*> _subfolders;
		std::vector<Node*> _files;

		Node* _parent;

		void _getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const;
	};
	Node* _root;
	Hydra::World::IWorld* _world;
	std::string _getExecutableDir();

};

