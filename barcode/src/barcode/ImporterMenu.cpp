#include <barcode/ImporterMenu.hpp>
#include <hydra/engine.hpp>
#include <hydra/component/cameracomponent.hpp>
#include <hydra/component/roomcomponent.hpp>

using world = Hydra::World::World;

ImporterMenu::ImporterMenu()
{
	this->executableDir = _getExecutableDir();
	this->_root = nullptr;
	this->_newEntityClicked = true;
	this->_rotation = 0.f;
	refresh();
}
ImporterMenu::~ImporterMenu()
{
	if(_root != nullptr)
	delete _root;
}
void ImporterMenu::render(bool &closeBool, Hydra::Renderer::Batch& previewBatch, float delta)
{
	ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiSetCond_Once);
	ImGui::Begin("Import", &closeBool, ImGuiWindowFlags_MenuBar);
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
	ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, ImGui::GetWindowContentRegionMax().y - 60));
	if(_root != nullptr)
		_root->render(&selectedNode);
	if (selectedNode != nullptr && selectedNode->getExt() == ".mATTIC") {
		std::string fileName = selectedNode->reverseEngineerPath();
		std::cout << fileName.c_str() << std::endl;

		if (previewBatch.objects.size() > 0) {
			if (_previewEntity->name.c_str() == fileName.c_str()) // FIXME: Wtf?
				_newEntityClicked = false;
			else
				_newEntityClicked = true;
		}

		previewBatch.objects.clear();
		// Because of issues with the engine adding components will always be added to the global world
		// So we need to disable the meshes to have them be only viewed in the previewWindow
		if (_newEntityClicked && selectedNode->getExt() == ".mATTIC") {
			_previewEntity = world::newEntity(fileName, world::invalidID); // invalidID - Will not show up in the entity list
			_previewEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh(fileName);
			auto drawObject = _previewEntity->getComponent<Hydra::Component::DrawObjectComponent>();
			drawObject->drawObject->disable = true; // Disable mesh to only see the model in preview window

			auto tc = _previewEntity->addComponent<Hydra::Component::TransformComponent>();
			auto cc = _previewEntity->addComponent<Hydra::Component::CameraComponent>();
			cc->position = glm::vec3{0, 0, 5};
			tc->setRotation(glm::angleAxis(glm::radians(_rotation), glm::vec3(0,1,0)));
		}
		auto drawObject = _previewEntity->getComponent<Hydra::Component::DrawObjectComponent>();
		auto cc = _previewEntity->getComponent<Hydra::Component::CameraComponent>();
		previewBatch.pipeline->setValue(0, cc->getViewMatrix());
		previewBatch.pipeline->setValue(1, cc->getProjectionMatrix());
		previewBatch.pipeline->setValue(2, cc->position);
		previewBatch.objects[drawObject->drawObject->mesh].push_back(drawObject->drawObject->modelMatrix);
	}

	ImGui::EndChild();
	ImGui::SameLine();
	Hydra::IEngine::getInstance()->getRenderer()->render(previewBatch);
	//Preview window
	ImGui::BeginChild("Preview", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.7f, ImGui::GetWindowContentRegionMax().y - 60));
	//ImGui::SetNextWindowPos(ImVec2(10, 10));
	//ImGui::SetNextWindowSize(ImVec2(20, 20));
	ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<Hydra::Renderer::IFramebuffer&>(*previewBatch.renderTarget)[0]->getID()), ImVec2(ImGui::GetWindowContentRegionWidth() * 1.2f, ImGui::GetWindowContentRegionMax().y - 15));
	ImGui::EndChild();
	ImGui::End();


	_rotation += delta * 1;
}
void ImporterMenu::refresh()
{
	if (_root != nullptr)
	{
		delete _root;
	}
	_root = new Node(executableDir + "/assets");
	_root->clean();
}
std::string ImporterMenu::_getExecutableDir()
{
	std::string path;
#ifdef _WIN32
	char unicodePath[MAX_PATH];
	int bytes = GetModuleFileName(NULL, unicodePath, MAX_PATH);
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
	size_t index = path.find_last_of('/');
	path.erase(path.begin() + index, path.end());
	return path;
}
std::shared_ptr<Entity> ImporterMenu::getRoomEntity() {
	std::vector<std::shared_ptr<Entity>> entities;
	world::getEntitiesWithComponents<Hydra::Component::TransformComponent, Hydra::Component::RoomComponent>(entities);
	if (entities.size() > 0)
	{
		return entities[0];
	}
	return nullptr;
}

ImporterMenu::Node::Node()
{
	this->_name = "";
	this->_subfolders = std::vector<Node*>(0);
	this->_files = std::vector<Node*>(0);
	this->_parent = nullptr;
}
ImporterMenu::Node::Node(std::string path, Node* parent, bool isFile)
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

ImporterMenu::Node::~Node()
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
std::string ImporterMenu::Node::name()
{
	return _name;
}
std::string ImporterMenu::Node::getExt()
{
	size_t i = _name.find_last_of('.');
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
	if (this->_parent != nullptr)
	{
		upperPath = _parent->reverseEngineerPath();
		return upperPath + "/" + this->_name;
	}
	return this->_name;
}
//Returns all files in this folder and subfolders
int ImporterMenu::Node::numberOfFiles()
{	
	if (!isAllowedFile)
	{
		size_t allFiles = _files.size();
		for (size_t i = 0; i < _subfolders.size(); i++)
		{
			allFiles += _subfolders[i]->numberOfFiles();
		}
		return (unsigned int)allFiles;
	}
	return -1;
}
//Removes all folders that do not have any files
void ImporterMenu::Node::clean()
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
void ImporterMenu::Node::render(Node** selectedNode)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	auto label = ICON_FA_FOLDER " %s";
	if (ImGui::TreeNodeEx(this, node_flags, label, _name.c_str()))
	{
		label = ICON_FA_FOLDER_OPEN " %s";
		if (ImGui::IsItemClicked())
		{
			(*selectedNode) = this;
		}
		for (size_t i = 0; i < this->_subfolders.size(); i++)
		{
			_subfolders[i]->render(selectedNode);
		}
		for (size_t i = 0; i < this->_files.size(); i++)
		{
			std::string ext = this->_files[i]->getExt();
			if (ext == ".mattic" || ext == ".mATTIC")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBE " %s", _files[i]->_name.c_str());
				if (ImGui::IsItemClicked())
				{
					(*selectedNode) = _files[i];
					if (ImGui::IsMouseDoubleClicked(0) && getRoomEntity() != nullptr)
					{
						std::shared_ptr<Hydra::World::Entity> newEntity = world::newEntity(_files[i]->name(), getRoomEntity().get());
						newEntity->addComponent<Hydra::Component::MeshComponent>()->loadMesh(_files[i]->reverseEngineerPath());
						newEntity->addComponent<Hydra::Component::TransformComponent>();
					}
				}
			}
			else if (ext == ".room" || ext == ".ROOM")
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, ICON_FA_CUBES " %s", _files[i]->_name.c_str());
				if (ImGui::IsItemClicked())
				{
					(*selectedNode) = _files[i];
					if (ImGui::IsMouseDoubleClicked(0))
					{
						if (getRoomEntity() != nullptr)
						{
							getRoomEntity().get()->dead = true;
						}
						auto room = world::newEntity("Room", world::root());
						auto bp = BlueprintLoader::load(_files[i]->reverseEngineerPath());
						bp->spawn(room);
					}
				}
			}
			else
			{
				ImGui::TreeNodeEx(_files[i], node_flags | ImGuiTreeNodeFlags_Leaf, ICON_FA_QUESTION_CIRCLE_O " %s", _files[i]->_name.c_str());
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
			size_t i = fileName.find_last_of('.');
			std::string fileExt = fileName.substr(i, fileName.size() - i);
			if (fileExt == ".mATTIC")
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
