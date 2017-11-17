#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <hydra/engine.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>
#include <imgui/icons.hpp>
#include <hydra/world/world.hpp>

#include <hydra/world/blueprintloader.hpp>
#include <hydra/renderer/uirenderer.hpp>
#include <hydra/renderer/glrenderer.hpp>
using world = Hydra::World::World;
class FileTree
{
public:
	std::string executableDir;
	FileTree();
	~FileTree();
	virtual void refresh(std::string relativePath);
	static std::shared_ptr<Hydra::World::Entity> getRoomEntity();
	virtual void render(bool &openBool, Hydra::Renderer::Batch* previewBatch = nullptr, float delta = 0.0f) = 0;
	
	class Node {
	public:
		bool isAllowedFile = false;
		bool openInFileExplorer = false;

		Node();
		Node(std::string path, const std::vector<std::string>& extWhitelist, Node* parent = nullptr, bool isFile = false);
		virtual ~Node();

		std::string name();
		std::string getExt();
		std::string pathToName(std::string path);
		std::string reverseEngineerPath();
		int numberOfFiles();
		void clean();
		void render(Node** selectedNode);
		void popupMenu(Node* currentNode);
	private:
		std::string _name;
		std::vector<Node*> _subfolders;
		std::vector<Node*> _files;
		Node* _parent;

		void _getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders, const std::vector<std::string> &extWhitelist) const;
	};
protected:
	Node* _root;
	std::string _getExecutableDir();
	std::vector<std::string> _extWhitelist;
	std::string _selectedPath;
	virtual void _menuBar();
	void openInExplorer(std::string path = "");
};
