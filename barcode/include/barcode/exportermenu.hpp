#pragma once
#include <barcode/filetree.hpp>
class ExporterMenu : public FileTree {
public:
	ExporterMenu();
	~ExporterMenu();

	void render(bool &closeBool, Hydra::Renderer::Batch* previewBatch = nullptr, float delta = 0.0f);
private:
	char _selectedFileName[128] = "";
	int _fileType = 0;
	const char* _exportTypes[2] = {".room",".prefab"};
	bool _prepExporting = false;

	void _renderEntitySelector();
};
