#pragma once
#include <barcode/filemanager.hpp>

class ImporterMenu : FileManager{
public:
	ImporterMenu();
	~ImporterMenu();

	void render(bool &closeBool, Hydra::Renderer::Batch& previewBatch, float delta);
	static std::shared_ptr<Entity> getRoomEntity();
private:
	std::shared_ptr<Hydra::World::Entity> _previewEntity;
	bool _newEntityClicked;
};
