#include <hydra/component/textcomponent.hpp>

Hydra::Component::TextComponent::~TextComponent(){
	renderingData.clear();
}

void Hydra::Component::TextComponent::serialize(nlohmann::json & json) const{

}

void Hydra::Component::TextComponent::deserialize(nlohmann::json & json){

}

void Hydra::Component::TextComponent::registerUI(){

}
