#pragma once
#include <string>
#include <vector>
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
static std::string executableDir;
static std::string assetsDir;
static std::string modelsDir;
class ImporterMenu
{
public:
	ImporterMenu();
	~ImporterMenu();
	void render(bool &closeBool);
	void refresh();
	class Node
	{
	public:
		std::vector<Node*> subfolders;
		std::vector<Node*> files;
		Node* parent;
		std::string path;

		Node(std::string path);
		~Node();

		std::string name();
		void render();
	private:
		void _getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const;
	};
private:
	std::string rootPath;
	Node* root;
	std::string _getExecutableDir();
};

