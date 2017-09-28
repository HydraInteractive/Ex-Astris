#include <barcode/ImporterMenu.hpp>

ImporterMenu::ImporterMenu()
{
	this->executableDir = "";
	this->root = nullptr;
	this->_world = nullptr;
}
ImporterMenu::ImporterMenu(Hydra::World::IWorld* world)
{
	this->executableDir = _getExecutableDir();
	this->root = nullptr;
	this->_world = world;
	refresh();
}
ImporterMenu::~ImporterMenu()
{
	if(root != nullptr)
	delete root;
}
void ImporterMenu::render(bool &closeBool)
{
	ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiSetCond_Once);
	ImGui::Begin("Import", &closeBool);
	if(root != nullptr)
	root->render(0, _world);
	ImGui::End();
}
void ImporterMenu::refresh()
{
	if (root != nullptr)
	{
		delete root;
	}
	root = new Node(executableDir + "/assets");
	root->clean();
}
std::string ImporterMenu::_getExecutableDir()
{
	std::string path;
#ifdef _WIN32
	char unicodePath[MAX_PATH];
	int bytes = GetModuleFileName(NULL, unicodePath, 500);
#else
	char unicodePath[1000];
	char tempStr[32];
	sprintf(tempStr, "/proc/%d/exe", getpid());
	int bytes = std::min((int)readlink(tempStr, unicodePath, 500), 500 - 1);
	if (bytes >= 0)
		unicodePath[bytes] = '\0';
#endif
	if (bytes == 0)
		return "/";

	path = std::string(unicodePath);
	std::replace(path.begin(), path.end(), '\\', '/');
	int index = path.find_last_of('/');
	path.erase(path.begin() + index, path.end());
	return path;
}

ImporterMenu::Node::Node()
{
	this->_name = "";
	this->subfolders = std::vector<Node*>(0);
	this->files = std::vector<Node*>(0);
	this->parent = nullptr;
}
ImporterMenu::Node::Node(std::string path, Node* parent, bool isFile)
{
	this->_name = pathToName(path);
	this->subfolders = std::vector<Node*>();
	this->files = std::vector<Node*>();
	this->parent = parent;
	this->isAllowedFile = isFile;

	std::vector<std::string> inFiles;
	std::vector<std::string> inFolders;
	_getContentsOfDir(path, inFiles, inFolders);
	for (size_t i = 0; i < inFolders.size(); i++)
	{
		this->subfolders.push_back(new Node(inFolders[i], this));
	}
	for (size_t i = 0; i < inFiles.size(); i++)
	{
		this->files.push_back(new Node(inFiles[i], this));
	}
}
ImporterMenu::Node::~Node()
{
	for (size_t i = 0; i < subfolders.size(); i++)
	{
		delete subfolders[i];
	}
	subfolders.clear();
	for (size_t i = 0; i < files.size(); i++)
	{
		delete files[i];
	}
	files.clear();
}
std::string ImporterMenu::Node::name()
{
	return _name;
}
std::string ImporterMenu::Node::getExt()
{
	int i = _name.find_last_of('.');
	std::string fileExt = _name.substr(i, _name.size() - i);
	return fileExt;
}
std::string ImporterMenu::Node::pathToName(std::string path)
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
std::string ImporterMenu::Node::reverseEngineerPath()
{
	std::string upperPath = "";
	if (this->parent != nullptr)
	{
		upperPath = parent->reverseEngineerPath();
		return upperPath + "/" + this->_name;
	}
	return this->_name;
}
//Returns all files in this folder and subfolders
int ImporterMenu::Node::numberOfFiles()
{	
	if (!isAllowedFile)
	{
		int allFiles = files.size();
		for (size_t i = 0; i < subfolders.size(); i++)
		{
			allFiles += subfolders[i]->numberOfFiles();
		}
		return allFiles;
	}
	return -1;
}
//Removes all folders that do not have any files
void ImporterMenu::Node::clean()
{
	for (size_t i = 0; i < subfolders.size(); i++)
	{
		if (subfolders[i]->numberOfFiles() == 0)
		{
			delete subfolders[i];
			subfolders.erase(subfolders.begin() + i);
			i--;
		}
		else
		{
			subfolders[i]->clean();
		}
	}
}
void ImporterMenu::Node::render(int index, Hydra::World::IWorld* world)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	//TODO: Folder icon opening
	if (ImGui::TreeNodeEx((void*)(intptr_t)index, node_flags, ICON_FA_FOLDER " %s", _name.c_str()))
	{	
		for (size_t i = 0; i < this->subfolders.size(); i++)
		{
			subfolders[i]->render(i, world);
		}
		for (size_t i = 0; i < this->files.size(); i++)
		{
			std::string ext = this->files[i]->getExt();
			if (ext == ".attic" || ext == ".ATTIC")
			{
				ImGui::TreeNodeEx(files[i], node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBE " %s", files[i]->_name.c_str());
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
				{
					Hydra::World::IEntity* newEntity = world->createEntity(files[i]->name()).get();
					newEntity->addComponent<Hydra::Component::MeshComponent>(files[i]->reverseEngineerPath());
					newEntity->addComponent<Hydra::Component::TransformComponent>(glm::vec3(0, 0, 0));
				}
			}
			else if (ext == ".json" || ext == ".JSON")
			{
				ImGui::TreeNodeEx(files[i], node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBES " %s", files[i]->_name.c_str());
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
				{
					std::ifstream inFile(files[i]->reverseEngineerPath());
					nlohmann::json json;
					world->getWorldRoot()->serialize(json, true);

					//Write to file in json format
					inFile >> json;

					world->getWorldRoot()->deserialize(json);
				}
			}
			else if (ext == ".png" || ext == ".PNG")
			{
				ImGui::TreeNodeEx(files[i], node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_FILE_IMAGE_O " %s", files[i]->_name.c_str());
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
				{
					//Do whatever we do with images
				}
			}
		}
		ImGui::TreePop();
	}
}
void ImporterMenu::Node::_getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const
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
		}
	}
	while (FindNextFile(dir, &fileData));

	FindClose(dir);
#else ///Unix
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
		else if(stat(fullFileName.c_str(), &st) != -1)
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
