#include "hydra/renderer/ImporterMenu.hpp"
ImporterMenu::ImporterMenu()
{
	rootPath = _getExecutableDir();
	int hello = 32423;
	//refresh();
}
ImporterMenu::~ImporterMenu()
{
	delete root;
}
void ImporterMenu::render(bool &closeBool)
{
	ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiSetCond_Once);
	ImGui::Begin("Static model", &closeBool);
	ImGui::End();
}
void ImporterMenu::refresh()
{
	delete root;
	root = new Node(rootPath);
}

std::string ImporterMenu::_getExecutableDir()
{	
	char unicodePath[MAX_PATH];
#ifdef _WIN32
	std::string path;
	int bytes = GetModuleFileName(NULL, unicodePath, 500);
#else
	sprintf(szTmp, "/proc/%d/exe", getpid());
	int bytes = MIN(readlink("/proc/self/exe", unicodePath, 500), len - 1);
	if (bytes >= 0)
		unicodePath[bytes] = '\0';
#endif
	if (bytes == 0)
		return "/";
	else
		//TODO: Will break if the path has unicode characters
		path = std::string(unicodePath);
	return path;
}

ImporterMenu::Node::Node(std::string path)
{
	std::vector<std::string> files;
	std::vector<std::string> folders;
	_getContentsOfDir(path, files, folders);
	int hello = 2;
}
ImporterMenu::Node::Node(std::string path, std::string children)
{

}
ImporterMenu::Node::~Node()
{

}
std::string ImporterMenu::Node::getFileName()
{
	int i = path.find_last_of('/');
	if (i == std::string::npos)
	{
		return path;
	}
	else
	{
		return path.substr(i + 1);
	}
}
void ImporterMenu::Node::_getContentsOfDir(const std::string &directory, std::vector<std::string> &files, std::vector<std::string> &folders) const
{
#ifdef _WIN32 ///Windows
	HANDLE dir;
	WIN32_FIND_DATA fileData;

	if ((dir = FindFirstFile((directory + "/*").c_str(), &fileData)) == INVALID_HANDLE_VALUE)
	{
		//No files found
		files.empty();
		folders.empty();
		return;
	}
	do
	{
		const std::string fileName = fileData.cFileName;
		const std::string fullFileName = directory + "/" + fileName;
		const bool is_directory = (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (fileName[0] == '.')
		{
			//files.push_back(fullFileName);
		}
		else if (is_directory)
		{
			folders.push_back(fullFileName);
		}
		else
		{
			files.push_back(fullFileName);
		}
	}
	while (FindNextFile(dir, &fileData));

	FindClose(dir);
#else ///Unix
	DIR *dir;
	class dirent *ent;
	class stat st;

	dir = opendir(directory);
	while ((ent = readdir(dir)) != NULL)
	{
		const string fileName = ent->d_name;
		const string fullFileName = directory + "/" + fileName;

		if (fileName[0] == '.')
			continue;

		if (stat(fullFileName.c_str(), &st) == -1)
			continue;

		const bool isDir = (st.st_mode & S_IFDIR) != 0;

		if (isDir)
			continue;

		out.push_back(fullFileName);
	}
	closedir(dir);
#endif
}
