#include <barcode/ExporterMenu.hpp>
ExporterMenu::ExporterMenu()
{
	this->executableDir = "";
	this->_root = nullptr;
	this->_world = nullptr;
	this->_selectedPath = "";
}
ExporterMenu::ExporterMenu(Hydra::World::IWorld* world)
{
	this->executableDir = _getExecutableDir();
	this->_root = nullptr;
	this->_world = world;
	this->_selectedPath = "";
	refresh();
}
ExporterMenu::~ExporterMenu()
{
	if (_root != nullptr)
		delete _root;
}
void ExporterMenu::render(bool &closeBool)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiSetCond_Once);
	ImGui::Begin("Export", &closeBool, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Refresh", NULL))
			{
				refresh();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	Node* selectedNode = nullptr;

	//File tree
	ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetWindowContentRegionMax().y - 60));
	if (_root != nullptr)
		_root->render(_world, &selectedNode, _prepExporting);
	ImGui::EndChild();

	ImGui::SameLine();

	//File name dialog
	ImGui::BeginChild("File", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetWindowContentRegionMax().y - 60));

	//Refresh changes from file tree
	if (selectedNode != nullptr)
	{
		_selectedPath = selectedNode->reverseEngineerPath();
		if (selectedNode->isAllowedFile)
		{
			_selectedPath.erase(_selectedPath.end() - selectedNode->name().length(), _selectedPath.end());

			std::string fileNameWithoutExt = selectedNode->name();
			fileNameWithoutExt.erase(fileNameWithoutExt.end() - selectedNode->getExt().length(), fileNameWithoutExt.end());
			//Force empty the string before copying the new one
			memset(_selectedFileName, 0, sizeof(_selectedFileName));
			strncpy(_selectedFileName, fileNameWithoutExt.c_str(), fileNameWithoutExt.length());
		}
		else
		{
			_selectedPath.append("/");
		}
	}

	//Draw gui
	ImGui::Text("Path: "); ImGui::SameLine(); ImGui::Text("%s", _selectedPath.c_str());
	ImGui::InputText(".room", _selectedFileName, 128);

	if (ImGui::Button("Save"))
	{
		_prepExporting = true;
	}
	if (_prepExporting)
	{
		std::string fileToSave = "";
		fileToSave.append(_selectedPath);
		fileToSave.append(_selectedFileName);
		fileToSave.append(".room");
		std::ifstream infile(fileToSave);
		bool doExport = false;
		if (infile.good())
		{
			ImGui::OpenPopup("Overwrite?");
		}
		else
		{
			doExport = true;
		}

		if (ImGui::BeginPopupModal("Overwrite?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			std::string textline = "This action will overwrite the file at " + fileToSave + ". Continue?";
			ImGui::Text("%s", textline.c_str());
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				doExport = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{	
				_prepExporting = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (doExport)
		{
			std::ofstream outFile;
			if (getRoomEntity(_world) != nullptr)
				BlueprintLoader::save(fileToSave, "Room", getRoomEntity(_world));
			_prepExporting = false;
			closeBool = false;
		}
	}
	ImGui::EndChild();
	ImGui::End();
}
void ExporterMenu::refresh()
{
	if (_root != nullptr)
	{
		delete _root;
	}
	_root = new Node(executableDir + "/assets");
	//_root->clean();
}
std::shared_ptr<IEntity> ExporterMenu::getRoomEntity(Hydra::World::IWorld* world)
{
	std::vector<std::shared_ptr<IEntity>> entities = world->getWorldRoot()->getChildren();
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i]->getName() == "Room")
			return entities[i];
	}
	return nullptr;
}
std::string ExporterMenu::_getExecutableDir()
{
	std::string path;
#ifdef _WIN32 ///Windows
	char unicodePath[MAX_PATH];
	int bytes = GetModuleFileName(NULL, unicodePath, 500);
#else ///Linux
	char unicodePath[1000];
	char tempStr[32];
	sprintf(tempStr, "/proc/%d/exe", getpid());
	int bytes = std::min((int)readlink(tempStr, unicodePath, 500), 500 - 1);
	if (bytes >= 0)
		unicodePath[bytes] = '\0';
#endif
	if (bytes == 0)
		return "/";
	else
		path = std::string(unicodePath);
	std::replace(path.begin(), path.end(), '\\', '/');
	int index = path.find_last_of('/');
	path.erase(path.begin() + index, path.end());
	return path;
}

ExporterMenu::Node::Node()
{
	this->_name = "";
	this->_subfolders = std::vector<Node*>(0);
	this->_files = std::vector<Node*>(0);
	this->_parent = nullptr;
}
ExporterMenu::Node::Node(std::string path, Node* parent, bool isFile)
{
	this->_name = pathToName(path);
	this->_subfolders = std::vector<Node*>();
	this->_files = std::vector<Node*>();
	this->_parent = parent;
	this->isAllowedFile = isFile;

	std::vector<std::string> inFiles;
	std::vector<std::string> inFolders;
	_getContentsOfDir(path, inFiles, inFolders);
	for (size_t i = 0; i < inFolders.size(); i++)
	{
		this->_subfolders.push_back(new Node(inFolders[i], this));
	}
	for (size_t i = 0; i < inFiles.size(); i++)
	{
		this->_files.push_back(new Node(inFiles[i], this, true));
	}
}
ExporterMenu::Node::~Node()
{
	for (size_t i = 0; i < _subfolders.size(); i++)
	{
		delete _subfolders[i];
	}
	_subfolders.clear();
	for (size_t i = 0; i < _files.size(); i++)
	{
		delete _files[i];
	}
	_files.clear();
}
std::string ExporterMenu::Node::name()
{
	return _name;
}
//Returns the file extention from a filename
std::string ExporterMenu::Node::getExt()
{
	size_t i = _name.find_last_of('.');
	if (i == std::string::npos)
	{
		return "";
	}
	else
	{
		std::string fileExt = _name.substr(i, _name.size() - i);
		return fileExt;
	}
}
//Gets the node's name from it's path
std::string ExporterMenu::Node::pathToName(std::string path)
{
	size_t i = path.find_last_of('/');
	if (i == std::string::npos)
	{
		return path;
	}
	else
	{
		return path.substr(i + 1);
	}
}
std::string ExporterMenu::Node::reverseEngineerPath()
{
	std::string upperPath = "";
	if (this->_parent != nullptr)
	{
		upperPath = _parent->reverseEngineerPath();
		return upperPath + "/" + this->_name;
	}
	return this->_name;
}
//Returns all files in this folder and subfolders
int ExporterMenu::Node::numberOfFiles()
{
	if (!isAllowedFile)
	{
		int allFiles = _files.size();
		for (size_t i = 0; i < _subfolders.size(); i++)
		{
			allFiles += _subfolders[i]->numberOfFiles();
		}
		return allFiles;
	}
	return -1;
}
//Removes all folders that do not have any files
void ExporterMenu::Node::clean()
{
	for (size_t i = 0; i < _subfolders.size(); i++)
	{
		if (_subfolders[i]->numberOfFiles() == 0)
		{
			delete _subfolders[i];
			_subfolders.erase(_subfolders.begin() + i);
			i--;
		}
		else
		{
			_subfolders[i]->clean();
		}
	}
}
void ExporterMenu::Node::render(Hydra::World::IWorld* world, Node** selectedNode, bool& prepExporting)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	//TODO: Folder icon opening
	if (ImGui::TreeNodeEx(this, node_flags, ICON_FA_FOLDER " %s", _name.c_str()))
	{	
		if (ImGui::IsItemClicked())
		{
			(*selectedNode) = this;
		}
		for (size_t i = 0; i < this->_subfolders.size(); i++)
		{
			_subfolders[i]->render(world, selectedNode, prepExporting);
		}
		for (size_t i = 0; i < this->_files.size(); i++)
		{
			std::string ext = this->_files[i]->getExt();
			if (ext == ".attic" || ext == ".ATTIC")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_CUBE " %s", _files[i]->_name.c_str());
			}
			else if (ext == ".room" || ext == ".ROOM")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_CUBES " %s", _files[i]->_name.c_str());
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
				{
					prepExporting = true;
				}
			}
			else
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_QUESTION_CIRCLE_O " %s", _files[i]->_name.c_str());
			}
			if (ImGui::IsItemClicked())
			{
				(*selectedNode) = _files[i];
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
void ExporterMenu::Node::_getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const
{
#ifdef _WIN32 ///Windows
	HANDLE dir;
	WIN32_FIND_DATA fileData;

	if ((dir = FindFirstFile((directory + "/*").c_str(), &fileData)) == INVALID_HANDLE_VALUE)
	{
		//No files found or is not directory
		files.empty();
		folders.empty();
		return;
	}
	do
	{
		const std::string fileName = fileData.cFileName;
		const std::string fullFilePath = directory + "/" + fileName;
		const bool is_directory = (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (fileName[0] == '.')
		{
			//files.push_back(fullFileName);
		}
		else if (is_directory)
		{
			folders.push_back(fullFilePath);
		}
		else
		{
			int i = fileName.find_last_of('.');
			std::string fileExt = fileName.substr(i, fileName.size() - i);
			if (fileExt == ".ATTIC")
			{
				files.push_back(fullFilePath);
			}
			else if (fileExt == ".json")
			{
				files.push_back(fullFilePath);
			}
			else if (fileExt == ".room")
			{
				files.push_back(fullFilePath);
			}
		}
	} while (FindNextFile(dir, &fileData));

	FindClose(dir);
#else ///Linux
	DIR *dir;
	class dirent *ent;
	class stat st;

	dir = opendir(directory.c_str());
	while ((ent = readdir(dir)) != NULL)
	{
		const std::string fileName = ent->d_name;
		const std::string fullFileName = directory + "/" + fileName;

		if (fileName[0] == '.')
		{

		}
		else if (stat(fullFileName.c_str(), &st) != -1)
		{
			const bool isDir = (st.st_mode & S_IFDIR) != 0;
			if (isDir)
			{
				folders.push_back(fullFileName);
			}
			else
			{
				files.push_back(fullFileName);
			}
		}
	}
	closedir(dir);
#endif
}
