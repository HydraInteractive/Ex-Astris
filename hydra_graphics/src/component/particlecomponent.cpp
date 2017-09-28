#include <hydra/component/particlecomponent.hpp>
#include <hydra/engine.hpp>
#include <imgui/imgui.h>
#include <hydra/component/cameracomponent.hpp>
#include <algorithm>

#define frand() (float(rand()) / float(RAND_MAX))
#define OUTERROW 8 // 24 or 6
// Outerrow decides how many particle textures there are.

using namespace Hydra::World;
using namespace Hydra::Component;

ParticleComponent::ParticleComponent(IEntity* entity) : IComponent(entity), _drawObject(entity->getDrawObject()), _pps(1), 
_behaviour(EmitterBehaviour::PerSecond), _accumulator(0.f), _emitterPos(glm::vec3(0)), _innerRow(4){
	_drawObject->refCounter++;
	_drawObject->mesh = nullptr;
}

ParticleComponent::ParticleComponent(IEntity* entity, EmitterBehaviour behaviour, ParticleTexture texture, int nrOfParticles, glm::vec3 pos) : IComponent(entity), _drawObject(entity->getDrawObject()),
_pps(nrOfParticles), _behaviour(behaviour), _accumulator(0.f), _emitterPos(pos), _innerRow(4){
	_drawObject->refCounter++;
	_drawObject->mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getQuad().get();
	_tempRotation = glm::mat4(1);
	switch (texture) {
	case ParticleTexture::Fire:
		_offsetToTexture = glm::ivec2(0, 0) * _innerRow; // First texture is 0 * 4, meaning top left in the texture. 
		break;
	case ParticleTexture::Knas:
		_offsetToTexture = glm::ivec2(1, 0) * _innerRow;
		break;
	case ParticleTexture::BogdanDeluxe:
		_offsetToTexture = glm::ivec2(2, 0) * _innerRow;
		break;
	}
}

ParticleComponent::~ParticleComponent() {
	_drawObject->refCounter--;
	_drawObject->mesh = nullptr;
	_particles.clear();
}

void ParticleComponent::tick(TickAction action, float delta){
	if (action == TickAction::physics && _particles.size() > 0)
		_particlePhysics(delta);
	if (action == TickAction::renderTransparent) {
		_accumulator += delta;
		_generateParticles();
		if (_particles.size() > 1)
			_sortParticles();
	}
}

void ParticleComponent::_generateParticles() {
	switch (_behaviour) {
	case EmitterBehaviour::PerSecond:
		if (_accumulator > 1.0f / _pps) {
			_emmitParticle();
			_accumulator -= 1.0f / _pps;
		}
		break;
	case EmitterBehaviour::Explosion:
		break;
	}
}

void ParticleComponent::_emmitParticle() {
	std::shared_ptr<Particle> p = std::make_shared<Particle>();
	float dirX = frand() * 20 - 10;
	float dirY = frand() * 20 - 10;
	float dirZ = frand() * 20 - 10;
	glm::vec3 a = glm::vec3(frand() * 4 - 2, frand() * 19.f - 8.5f, frand() * 6 - 3.f);
	p->spawn(_emitterPos, glm::normalize(glm::vec3(dirX, dirY, dirZ)), a, frand() * 1.f + 1.f);
	_particles.push_back(p);
}

void ParticleComponent::_particlePhysics(float delta) {
	auto camera = Hydra::IEngine::getInstance()->getState()->getWorld()
		->getActiveComponents<CameraComponent>()[0]
		->getComponent<CameraComponent>();

	for (auto& p : _particles) {
		if (p->life <= p->elapsedTime) {
			p->dead = true;
			continue;
		}

		if (camera)
			p->distanceToCamera = glm::distance(camera->getPosition(), p->pos);

		p->vel += p->acceleration * delta;
		p->pos += p->vel * delta;
		_updateTextureCoordInfo(p, delta);
		p->elapsedTime += delta;
		p->fixMX(_tempRotation);
	}
	_clearDeadParticles();
}

void ParticleComponent::_updateTextureCoordInfo(std::shared_ptr<Particle>& p, float delta) {
	float lifeFactor = p->elapsedTime / p->life;
	int stageCount = OUTERROW * OUTERROW;
	float atlasProg = stageCount * lifeFactor;
	int index1 = (int)floor(atlasProg);
	int index2 = index1 < stageCount - 1 ? index1 + 1 : index1;
	p->texCoordInfo = glm::vec2(OUTERROW, fmod(atlasProg, 1));
	_setTextureOffset(p->texOffset1, index1);
	_setTextureOffset(p->texOffset2, index2);
}

void ParticleComponent::_setTextureOffset(glm::vec2& offset, int index) {
	int column = index % OUTERROW;
	int row = index / OUTERROW;
	offset.x = column / (float)OUTERROW;
	offset.y = row / (float)OUTERROW;
}

void ParticleComponent::_clearDeadParticles() {
	_particles.erase(std::remove_if(
			_particles.begin(),
			_particles.end(),
			[](std::shared_ptr<Particle>& p) {
				return p->dead;
			}
		), 
	_particles.end()
	);
}

void ParticleComponent::_sortParticles() { // Insertion Sort
	int j;
	std::shared_ptr<Particle> temp;

	for (int i = 0; i < _particles.size(); i++) {
		j = i;
		while (j > 0 && _particles[j]->distanceToCamera > _particles[j-1]->distanceToCamera) {
			temp = _particles[j];
			_particles[j] = _particles[j - 1];
			_particles[j - 1] = temp;
			j--;
		}
	}
}

void ParticleComponent::serialize(nlohmann::json & json) const{
	json = {
		{ "pps", _pps},
		{ "accumulator", _accumulator},
		{ "behaviour", (int)_behaviour},
		{ "emitterPos", { _emitterPos.x, _emitterPos.y, _emitterPos.z } },
		{ "innerRow", _innerRow },
		{ "offsetToTexture", { _offsetToTexture.x, _offsetToTexture.y} }
	};
}

void ParticleComponent::deserialize(nlohmann::json & json){
	auto& pps = json["pps"];
	 _pps = pps.get<int>();

	auto& accumulator = json["accumulator"];
	_accumulator = accumulator.get<int>();

	auto& behaviour = json["behaviour"];
	_behaviour = (EmitterBehaviour)behaviour.get<int>();

	auto& pos = json["emitterPos"];
	_emitterPos = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };
	
	auto& innerRow = json["innerRow"];
	_innerRow = innerRow.get<int>();

	auto& offsetToTexture = json["offsetToTexture"];
	_offsetToTexture = glm::ivec2(offsetToTexture[0].get<int>(), offsetToTexture[1].get<int>());
	
	_drawObject->mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getQuad().get();
	_tempRotation = glm::mat4(1);
	_tempRotation *= glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1));
}

void ParticleComponent::registerUI(){
	ImGui::DragInt("ParticlesPerSecond", &_pps, 1.0);
}