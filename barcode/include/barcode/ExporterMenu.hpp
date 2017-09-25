#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
#include <imgui/icons.hpp>
#include <hydra/world/world.hpp>

#include <string>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
class ExporterMenu
{
public:
	std::string executableDir;
	ExporterMenu();
	ExporterMenu(Hydra::World::IWorld* world);
	~ExporterMenu();

	void render(bool &closeBool);
	void refresh();
private:
	class Node
	{
	public:
		Node();
		Node(std::string path, Node* parent = nullptr, bool isFile = false);
		~Node();

		std::string name();
		std::string getExt();
		std::string pathToName(std::string path);
		std::string reverseEngineerPath();
		int numberOfFiles();
		void clean();
		void render(size_t index, Hydra::World::IWorld* world);
	private:
		std::string _name;
		std::vector<Node*> subfolders;
		std::vector<Node*> files;
		bool isAllowedFile;
		Node* parent;

		void _getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const;
	};
	Node* root;
	Hydra::World::IWorld* _world;
	std::string _getExecutableDir();
};
