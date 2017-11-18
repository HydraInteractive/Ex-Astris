#pragma once
#include <hydra/ext/api.hpp>

#include <memory>
#include <map>
#include <hydra/renderer/renderer.hpp>

using namespace Hydra::Renderer;

namespace Hydra::IO {
	class HYDRA_BASE_API ITextFactory {
	public:
		struct CharInfo {
			glm::vec2 pos;
			glm::vec2 size;

			// glm::vec2 offset;
			float xAdvanceAmount;
		};

		virtual ~ITextFactory() = 0;
		virtual const CharInfo& getChar(char ch) = 0;
		virtual std::shared_ptr<ITexture> getTexture() = 0;

	};
	inline ITextFactory::~ITextFactory() {}
};
