#pragma once
#include <barcode/filemanager.hpp>
class ExporterMenu : FileManager{
public:
	ExporterMenu();
	~ExporterMenu();

	void render(bool &closeBool);
	static std::shared_ptr<Entity> getRoomEntity();
private:
	std::string _selectedPath;
	char _selectedFileName[128] = "";
	bool _prepExporting = false;
};