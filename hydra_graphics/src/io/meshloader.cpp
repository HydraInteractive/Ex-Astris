#include <hydra/io/glmeshloader.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/engine.hpp>

#include <exception>

using namespace Hydra;
using namespace Hydra::IO;

class MeshLoaderImpl final : public IMeshLoader {
public:
	MeshLoaderImpl(IRenderer* renderer) : _renderer(renderer), _errorMesh(_loadErrorMesh()) {}

	~MeshLoaderImpl() final {
		_storage.clear();
	}

	std::shared_ptr<IMesh> getMesh(const std::string& file) final {
		if (file.empty())
			return _errorMesh;
		std::shared_ptr<IMesh> mesh = _storage[file];
		if (!mesh) {
			try {
				IEngine::getInstance()->log(LogLevel::verbose, "Loading mesh: %s", file.c_str());
				mesh = _storage[file] = Hydra::Renderer::GLMesh::create(file, _renderer);
			}
			catch (const std::exception& e) {
				IEngine::getInstance()->log(LogLevel::error, "FAILED TO LOAD MESH: %s", e.what());
				return _errorMesh;
			}
		}

		return mesh;
	}

	std::shared_ptr<IMesh> getQuad() {
		std::shared_ptr<IMesh> mesh;
		mesh = Hydra::Renderer::GLMesh::createQuad(_renderer);
		return mesh;
	}

	std::shared_ptr<IMesh> getErrorMesh() final { return _errorMesh; }

private:
	IRenderer* _renderer;
	std::map<std::string, std::shared_ptr<IMesh>> _storage;
	std::shared_ptr<IMesh> _errorMesh;

	std::shared_ptr<IMesh> _loadErrorMesh() {
		//return Hydra::Renderer::GLMesh::create("assets/meshs/error.fbx");
		return std::shared_ptr<IMesh>(); // XXX: 
	}
};

std::unique_ptr<IMeshLoader> GLMeshLoader::create(IRenderer* renderer) {
	return std::unique_ptr<IMeshLoader>(new MeshLoaderImpl(renderer));
}