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

template< typename ContainerT, class _FwdIt, class _Pr >
void erase_if(ContainerT& items, _FwdIt it, _FwdIt _Last, _Pr _Pred) {
	for (; it != _Last; ) {
		if (_Pred(*it)) it = items.erase(it);
		else ++it;
	}
};

class MeshLoaderImpl final : public IMeshLoader {
public:
	MeshLoaderImpl(IRenderer* renderer) : _renderer(renderer), _errorMesh(_loadErrorMesh()) {}

	~MeshLoaderImpl() final {
		_storage.clear();
	}

	std::shared_ptr<IMesh> getMesh(const std::string& file) final {
		if (file.empty())
			return _errorMesh;
		if (file == "PARTICLEQUAD")
			return getParticleQuad();
		if (file == "TEXTQUAD")
			return getTextQuad();

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

	std::shared_ptr<IMesh> getParticleQuad() final{
		std::shared_ptr<IMesh> mesh = _storage["ParticleQuad"];
		if (!mesh)
			mesh = _storage["ParticleQuad"] = GLMesh::createParticleQuad(_renderer);
		return mesh;
	}

	std::shared_ptr<IMesh> getTextQuad() final {
		std::shared_ptr<IMesh> mesh = _storage["TextQuad"];
		if (!mesh)
			mesh = _storage["TextQuad"] = GLMesh::createTextQuad(_renderer);
		return mesh;
	}

	std::shared_ptr<IMesh> getErrorMesh() final { return _errorMesh; }

	void clear() {
		erase_if(_storage, _storage.begin(), _storage.end(), [](const std::pair<std::string, std::shared_ptr<IMesh>>& x) {
			return x.second.use_count() == 2;
		});
	}

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
