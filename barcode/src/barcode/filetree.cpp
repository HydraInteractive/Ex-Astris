#include <barcode/filetree.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/component/roomcomponent.hpp>
#include <memory>
FileTree::FileTree()
{
	this->executableDir = _getExecutableDir();
	this->_root = nullptr;
}

FileTree::~FileTree()
{
	if (_root != nullptr)
		delete _root;
}
void FileTree::refresh(std::string relativePath)
{
	if (_root != nullptr)
	{
		delete _root;
	}
	_root = new Node(executableDir + relativePath, extWhitelist);
}
std::shared_ptr<Hydra::World::Entity> FileTree::getRoomEntity()
{
	std::vector<std::shared_ptr<Hydra::World::Entity>> entities;
	world::getEntitiesWithComponents<Hydra::Component::TransformComponent, Hydra::Component::RoomComponent>(entities);
	if (entities.size() > 0)
	{
		return entities[0];
	}
	return nullptr;
}
std::string FileTree::_getExecutableDir()
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
FileTree::Node::Node()
{
	this->_name = "";
	this->_subfolders = std::vector<Node*>(0);
	this->_files = std::vector<Node*>(0);
	this->_parent = nullptr;
}
FileTree::Node::Node(std::string path, const std::vector<std::string>& extWhitelist, Node* parent, bool isFile)
{
	this->_name = pathToName(path);
	this->_subfolders = std::vector<Node*>();
	this->_files = std::vector<Node*>();
	this->_parent = parent;
	this->isAllowedFile = isFile;

	std::vector<std::string> inFiles;
	std::vector<std::string> inFolders;
	_getContentsOfDir(path, inFiles, inFolders, extWhitelist);
	for (size_t i = 0; i < inFolders.size(); i++)
	{
		this->_subfolders.push_back(new Node(inFolders[i], extWhitelist, this));
	}
	for (size_t i = 0; i < inFiles.size(); i++)
	{
		this->_files.push_back(new Node(inFiles[i], extWhitelist, this, true));
	}
}
FileTree::Node::~Node()
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
std::string FileTree::Node::name()
{
	return _name;
}
void FileTree::openInExplorer(LPCTSTR filename)
{
	ITEMIDLIST *pidl = ILCreateFromPath(filename);
	if (pidl) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
}
//Returns the file extention from a filename
std::string FileTree::Node::getExt()
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
std::string FileTree::Node::pathToName(std::string path)
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
std::string FileTree::Node::reverseEngineerPath()
{
	std::string upperPath = "";
	if (this->_parent != nullptr)
	{
		upperPath = _parent->reverseEngineerPath();
		return upperPath + "/" + this->_name;
	}
	return this->_name;
}

int FileTree::Node::numberOfFiles()
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
void FileTree::Node::clean()
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
//TODO: Add file ext whitelist
void FileTree::Node::_getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders, const std::vector<std::string> &extWhitelist) const
{
#ifdef _WIN32 ///Windows
	HANDLE dir;
	WIN32_FIND_DATA fileData;

	if ((dir = FindFirstFile((directory + "/*").c_str(), &fileData)) == INVALID_HANDLE_VALUE)
	{
		//No files found or is not directory
		files.clear();
		folders.clear();
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
			for (size_t i = 0; i < extWhitelist.size(); i++)
			{
				if (fileExt == extWhitelist[i])
				{
					files.push_back(fullFilePath);
				}
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