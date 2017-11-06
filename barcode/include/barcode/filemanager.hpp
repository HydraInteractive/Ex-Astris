#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
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
class FileManager
{
public:
	std::string executableDir;
	FileManager();
	~FileManager();
	void refresh(std::string relativePath);
	static std::shared_ptr<Hydra::World::Entity> getRoomEntity();
	virtual void render(bool &closeBool, Hydra::Renderer::Batch* previewBatch = nullptr, float delta = 0.0f) = 0;
	virtual class Node {
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
		virtual void render(Node** selectedNode, bool& prepExporting);
		virtual void render(Node** selectedNode);
	private:
		std::string _name;
		std::vector<Node*> _subfolders;
		std::vector<Node*> _files;
		Node* _parent;

		void _getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const;
	};
protected:
	Node* _root;
	std::string _getExecutableDir();
};