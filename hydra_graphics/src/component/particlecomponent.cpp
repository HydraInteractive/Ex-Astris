#include "hydra/component/particlecomponent.hpp"
#include <hydra/engine.hpp>

using namespace Hydra::World;
using namespace Hydra::Component;

ParticleComponent::ParticleComponent(IEntity* entity) : IComponent(entity), _drawObject(entity->getDrawObject()) {
	_drawObject->refCounter++;
	_drawObject->mesh = nullptr;
}

ParticleComponent::ParticleComponent(IEntity* entity, EmitterBehaviour behaviour, int nrOfParticles) : IComponent(entity), _drawObject(entity->getDrawObject()), _nrOfParticles(nrOfParticles), _behaviour(behaviour){
	_drawObject->refCounter++;
	_drawObject->mesh = Hydra::IEngine::getInstance()->getMeshLoader()->getQuad().get();
}

ParticleComponent::~ParticleComponent() {
	
}

void Hydra::Component::ParticleComponent::tick(TickAction action){

}

void Hydra::Component::ParticleComponent::serialize(nlohmann::json & json) const{

}

void Hydra::Component::ParticleComponent::deserialize(nlohmann::json & json){

}

void Hydra::Component::ParticleComponent::registerUI(){

}
