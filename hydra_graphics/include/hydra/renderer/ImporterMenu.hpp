#pragma once
#include <string>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
#include <imgui/icons.hpp>
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
		std::vector<Node*> children;
		Node* parent;
		std::string path;
		Node(std::string path);
		Node(std::string path, std::string children);
		~Node();
		std::string getFileName();
	private:
		void _getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const;
	};
private:
	std::string rootPath;
	Node* root;
	std::string _getExecutableDir();
};

