/**
 * A mesh loader that caches the loaded mesh, required for the instances rendering
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

//TODO: Use PBO! http://www.songho.ca/opengl/gl_pbo.html#unpack

namespace Hydra::IO {
	class HYDRA_BASE_API IMeshLoader {
	public:
		virtual ~IMeshLoader() = 0;

		virtual std::shared_ptr<IMesh> getMesh(const std::string& file) = 0;
		virtual std::shared_ptr<IMesh> getParticleQuad() = 0;
		virtual std::shared_ptr<IMesh> getTextQuad() = 0;
		virtual std::shared_ptr<IMesh> getErrorMesh() = 0;
		virtual void clear() = 0;
	};
	inline IMeshLoader::~IMeshLoader() {}
};
