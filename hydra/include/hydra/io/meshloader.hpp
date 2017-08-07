#pragma once

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

//TODO: Use PBO! http://www.songho.ca/opengl/gl_pbo.html#unpack

namespace Hydra::IO {
	class MeshLoader final {
	public:
		MeshLoader();
		virtual ~MeshLoader();

		std::shared_ptr<IMesh> getMesh(const std::string& file);
		std::shared_ptr<IMesh> getErrorMesh();

	private:
		std::map<std::string, std::shared_ptr<IMesh>> _storage;
		std::shared_ptr<IMesh> _errorMesh;

		std::shared_ptr<IMesh> _loadErrorMesh();
	};
};
