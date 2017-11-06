#pragma once
#include <barcode/filetree.hpp>
class ImporterMenu : public FileTree {
public:
	ImporterMenu();
	~ImporterMenu();

	void render(bool &closeBool, Hydra::Renderer::Batch* previewBatch, float delta);
private:
	std::shared_ptr<Hydra::World::Entity> _previewEntity;
	bool _newEntityClicked;
};
