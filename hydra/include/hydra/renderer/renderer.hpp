#pragma once

class Hydra::Entity;

namespace Hydra::Renderer {
	class IRenderer {
	public:
		virtual ~IRenderer() = 0;
		virtual void render(Entity* entity) = 0;
	};

	IRenderer::~IRenderer() {}
}
