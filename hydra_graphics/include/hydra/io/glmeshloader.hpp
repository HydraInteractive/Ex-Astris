#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <hydra/io/meshloader.hpp>

namespace Hydra::IO {
	namespace GLMeshLoader {
		HYDRA_API std::unique_ptr<IMeshLoader> create(IRenderer* renderer);
	};
};