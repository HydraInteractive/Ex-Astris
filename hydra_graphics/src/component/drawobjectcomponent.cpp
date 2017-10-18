#include <hydra/component/drawobjectcomponent.hpp>

#include <imgui/imgui.h>
#include <hydra/engine.hpp>

using namespace Hydra::Component;

DrawObjectComponent::DrawObjectComponent() {
	drawObject = Hydra::IEngine::getInstance()->getRenderer()->aquireDrawObject();
	drawObject->refCounter++;
}

DrawObjectComponent::~DrawObjectComponent() {
	drawObject->refCounter--;
}

void DrawObjectComponent::serialize(nlohmann::json& json) const {}

void DrawObjectComponent::deserialize(nlohmann::json& json) {}

void DrawObjectComponent::registerUI() {
	ImGui::Checkbox("Disable", &drawObject->disable);
}
