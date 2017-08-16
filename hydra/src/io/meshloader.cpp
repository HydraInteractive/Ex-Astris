#include <hydra/io/meshloader.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/engine.hpp>

#include <exception>

using namespace Hydra;
using namespace Hydra::IO;

MeshLoader::MeshLoader(IRenderer* renderer) : _renderer(renderer), _errorMesh(_loadErrorMesh()) {}

MeshLoader::~MeshLoader() {
	_storage.clear();
}

std::shared_ptr<IMesh> MeshLoader::getMesh(const std::string& file) {
	if (file.empty())
		return _errorMesh;
	std::shared_ptr<IMesh> mesh = _storage[file];
	if (!mesh) {
		try {
			IEngine::getInstance()->log(LogLevel::verbose, "Loading mesh: %s", file.c_str());
			mesh = _storage[file] = Hydra::Renderer::GLMesh::create(file, _renderer);
		} catch (const std::exception& e) {
			IEngine::getInstance()->log(LogLevel::error, "FAILED TO LOAD MESH: %s", e.what());
			return _errorMesh;
		}
	}

	return mesh;
}


std::shared_ptr<IMesh> MeshLoader::getErrorMesh() {
	return _errorMesh;
}

std::shared_ptr<IMesh> MeshLoader::_loadErrorMesh() {
	//return Hydra::Renderer::GLMesh::create("assets/meshs/error.fbx");

	return std::shared_ptr<IMesh>(); // XXX: 
}
