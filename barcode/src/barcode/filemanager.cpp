#include <barcode/filemanager.hpp>
#include <hydra/component/meshcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
FileManager::FileManager()
{
	this->executableDir = _getExecutableDir();
	this->_root = nullptr;
	refresh("/assets");
}

FileManager::~FileManager()
{
	if (_root != nullptr)
		delete _root;
}
void FileManager::refresh(std::string relativePath)
{
	if (_root != nullptr)
	{
		delete _root;
	}
	_root = new Node(executableDir + relativePath);
	//_root->clean();
}
std::string FileManager::_getExecutableDir()
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
//Gets the node's name from it's path
std::string FileManager::Node::pathToName(std::string path)
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
std::string FileManager::Node::reverseEngineerPath()
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
int FileManager::Node::numberOfFiles()
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
void FileManager::Node::clean()
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
void FileManager::Node::_getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const
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