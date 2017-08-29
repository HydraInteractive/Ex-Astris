#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

//TODO: Use PBO! http://www.songho.ca/opengl/gl_pbo.html#unpack

namespace Hydra::IO {
	class HYDRA_API MeshLoader final {
	public:
		MeshLoader(IRenderer* renderer);
		virtual ~MeshLoader();

		std::shared_ptr<IMesh> getMesh(const std::string& file);
		std::shared_ptr<IMesh> getErrorMesh();

	private:
		IRenderer* _renderer;
		std::map<std::string, std::shared_ptr<IMesh>> _storage;
		std::shared_ptr<IMesh> _errorMesh;

		std::shared_ptr<IMesh> _loadErrorMesh();
	};
};
