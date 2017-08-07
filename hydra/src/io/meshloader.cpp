#include <hydra/io/meshloader.hpp>

#include <hydra/renderer/glrenderer.hpp>

#include <exception>
#include <iostream>

using namespace Hydra::IO;

MeshLoader::MeshLoader() : _errorMesh(_loadErrorMesh()) {}

MeshLoader::~MeshLoader() {
	_storage.clear();
}

std::shared_ptr<IMesh> MeshLoader::getMesh(const std::string& file) {
	if (file.empty())
		return _errorMesh;
	std::shared_ptr<IMesh> mesh = _storage[file];
	if (!mesh) {
		try {
			std::cout << "Loading mesh: " << file << std::endl;
			mesh = _storage[file] = Hydra::Renderer::GLMesh::create(file);
		} catch (const std::exception& e) {
			std::cerr << "FAILED TO LOAD MESH: " << e.what() << std::endl;
			return _errorMesh;
		} catch (const char* msg) {
			std::cerr << "FAILED TO LOAD MESH: " << msg << std::endl;
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
