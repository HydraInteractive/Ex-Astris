#include <hydra/component/textcomponent.hpp>
#include <imgui/imgui.h>

Hydra::Component::TextComponent::~TextComponent(){
	renderingData.clear();
}

void Hydra::Component::TextComponent::serialize(nlohmann::json & json) const{
	json["text"] = text;
	json["maxLength"] = maxLength;
	json["color"] = {color.x, color.y, color.z};
}

void Hydra::Component::TextComponent::deserialize(nlohmann::json & json){
	text = json.value<std::string>("text", "BigBoi");
	maxLength = json.value<size_t>("maxLength", 24);
	auto& jColor = json["color"];
	color = glm::vec3(jColor[0].get<float>(), jColor[1].get<float>(), jColor[2].get<float>());
}

void Hydra::Component::TextComponent::registerUI(){
	static char* buffer = (char*)text.c_str();
	if (ImGui::InputText("Text", buffer, maxLength)) {
		text = buffer;
		rebuild();
	}
}

void Hydra::Component::TextComponent::rebuild() {
	if (!IEngine::getInstance())
		return;
	renderingData.resize(text.size());
	auto factory = IEngine::getInstance()->getState()->getTextFactory();
	glm::vec3 pos{ 0 };
	for (size_t i = 0; i < text.size(); i++) {
		const Hydra::IO::ITextFactory::CharInfo& info = factory->getChar(text[i]);
		pos.x -= info.xAdvanceAmount * 1.2;
	}
	pos /= -2;

	for (size_t i = 0; i < text.size(); i++) {
		const Hydra::IO::ITextFactory::CharInfo& info = factory->getChar(text[i]);
		CharRenderInfo& rInfo = renderingData[i];
		rInfo.charRect = glm::vec4{info.pos, info.size};
		rInfo.charPos = pos;
		pos.x -= info.xAdvanceAmount * 1.2;
	}
}

void  Hydra::Component::TextComponent::setText(const std::string& inText) {
	if (inText.size() > maxLength)
		text = inText.substr(0, maxLength);
	else
		text = inText;

	rebuild();
}
