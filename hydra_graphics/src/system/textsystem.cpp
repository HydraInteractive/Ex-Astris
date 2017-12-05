#include <hydra/system/textsystem.hpp>
#include <hydra/component/textcomponent.hpp>
#include <hydra/component/transformcomponent.hpp>
#include <hydra/ext/openmp.hpp>
#include <hydra/engine.hpp>

using namespace Hydra::System;

Hydra::System::TextSystem::TextSystem(){

}

Hydra::System::TextSystem::~TextSystem(){

}

void Hydra::System::TextSystem::tick(float delta){
	Hydra::World::World::getEntitiesWithComponents<Hydra::Component::TextComponent>(entities);
	for (int_openmp_t i = 0; i < (int_openmp_t)entities.size(); i++) {
		auto textC = entities[i]->getComponent<Hydra::Component::TextComponent>();
		auto transC = entities[i]->getComponent<Hydra::Component::TransformComponent>();

		if (!textC->isStatic) {
			transC->position.y += 5.0f * delta;
			transC->setPosition(transC->position);
		}
	}
	entities.clear();
}

void Hydra::System::TextSystem::registerUI(){

}
