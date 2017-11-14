#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <hydra/io/textfactory.hpp>

namespace Hydra::IO {
	namespace GLTextFactory {
		HYDRA_GRAPHICS_API std::unique_ptr<ITextFactory> create();
	};
}