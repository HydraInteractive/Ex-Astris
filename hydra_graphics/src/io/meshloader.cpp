// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * An implementation for the IMeshLoader, using OpenGL.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/io/glmeshloader.hpp>

#include <hydra/renderer/glrenderer.hpp>
#include <hydra/engine.hpp>

#include <exception>

using namespace Hydra;
using namespace Hydra::IO;
using namespace Hydra::Renderer;

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
				mesh = _storage[file] = GLMesh::create(file, _renderer);
			}
			catch (const std::exception& e) {
				IEngine::getInstance()->log(LogLevel::error, "FAILED TO LOAD MESH: %s", e.what());
				return _errorMesh;
			}
		}

		return mesh;
	}

	std::shared_ptr<IMesh> getQuad() final{
		std::shared_ptr<IMesh> mesh = GLMesh::createFullscreenQuad();
		return mesh;
	}

	std::shared_ptr<IMesh> getErrorMesh() final { return _errorMesh; }

private:
	IRenderer* _renderer;
	std::map<std::string, std::shared_ptr<IMesh>> _storage;
	std::shared_ptr<IMesh> _errorMesh;

	std::shared_ptr<IMesh> _loadErrorMesh() {
		//return GLMesh::create("assets/meshs/error.fbx");
		return std::shared_ptr<IMesh>(); // XXX: 
	}
};

std::unique_ptr<IMeshLoader> GLMeshLoader::create(IRenderer* renderer) {
	return std::unique_ptr<IMeshLoader>(new MeshLoaderImpl(renderer));
}
