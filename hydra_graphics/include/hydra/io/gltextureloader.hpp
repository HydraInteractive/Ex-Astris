#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <hydra/io/textureloader.hpp>

namespace Hydra::IO {
	namespace GLTextureLoader {
		HYDRA_API std::unique_ptr<ITextureLoader> create();
	};
};