#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

namespace Hydra::IO {
	class HYDRA_BASE_API ITextFactory {
	public:
		virtual ~ITextFactory() = 0;

	};
	inline ITextFactory::~ITextFactory() {}
};
