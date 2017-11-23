#include <hydra/component/drawobjectcomponent.hpp>

#include <imgui/imgui.h>
#include <hydra/engine.hpp>

using namespace Hydra::Component;

DrawObjectComponent::DrawObjectComponent() {
	if (Hydra::IEngine::getInstance()) { //Hax-Fix
		drawObject = Hydra::IEngine::getInstance()->getRenderer()->aquireDrawObject();
		drawObject->refCounter = 1;
	}
}

DrawObjectComponent::~DrawObjectComponent() {
	drawObject->refCounter = 0;
}

void DrawObjectComponent::serialize(nlohmann::json& json) const {}

void DrawObjectComponent::deserialize(nlohmann::json& json) {}

void DrawObjectComponent::registerUI() {
	ImGui::Checkbox("Disable", &drawObject->disable);
}
