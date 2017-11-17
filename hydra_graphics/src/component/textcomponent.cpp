#include <hydra/component/textcomponent.hpp>
#include <imgui/imgui.h>

Hydra::Component::TextComponent::~TextComponent(){
	renderingData.clear();
}

void Hydra::Component::TextComponent::serialize(nlohmann::json & json) const{
	json["text"] = text;
	json["maxLength"] = maxLength;
}

void Hydra::Component::TextComponent::deserialize(nlohmann::json & json){
	text = json.value<std::string>("text", "BigBoi");
	maxLength = json.value<int>("maxLength", 10);
}

void Hydra::Component::TextComponent::registerUI(){
	static char* buffer = (char*)text.c_str();
	if (ImGui::InputText("Text", buffer, maxLength)) {
		text = buffer;
		rebuild();
	}
}
