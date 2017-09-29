#include <hydra/component/particlecomponent.hpp>
#include <hydra/engine.hpp>
#include <imgui/imgui.h>
#include <hydra/component/cameracomponent.hpp>
#include <algorithm>

#define frand() (float(rand()) / float(RAND_MAX))
#define OUTERROW 6 // 24 or 6
#define INNER_ROW 4
// Outerrow decides how many particle textures there are.

using namespace Hydra::World;
using namespace Hydra::Component;

ParticleComponent::ParticleComponent(IEntity* entity) : IComponent(entity), _drawObject(entity->getDrawObject()), _pps(1), 
_behaviour(EmitterBehaviour::PerSecond), _accumulator(0.f), _emitterPos(glm::vec3(0)) {
	_drawObject->refCounter++;
	_drawObject->mesh = nullptr;
}

ParticleComponent::ParticleComponent(IEntity* entity, EmitterBehaviour behaviour, ParticleTexture texture, int nrOfParticles, glm::vec3 pos) : IComponent(entity), _drawObject(entity->getDrawObject()),
_pps(nrOfParticles), _behaviour(behaviour), _accumulator(0.f), _emitterPos(pos) {
	_drawObject->refCounter++;
	_drawObject->mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getQuad().get();
	_tempRotation = glm::mat4(1);

	size_t id = static_cast<size_t>(texture);
	_offsetToTexture = glm::vec2(id % OUTERROW, id / OUTERROW) / (float) OUTERROW;
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
	float dirX = frand() * 2 - 2;
	float dirY = frand() * -6 - 1;
	float dirZ = 0;
	glm::vec3 a = glm::vec3(frand() * 2 - 2, frand() * -8.5f, 0);
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
	const int innerCount = INNER_ROW * INNER_ROW;

	const float smallImageSize = 1.0f / (INNER_ROW * OUTERROW);

	const int innerID = innerCount * lifeFactor;
	const int nextInnerID = (innerID + 1) % INNER_ROW;

	const glm::vec2 outerOffset = _offsetToTexture;

	p->texOffset1 = outerOffset + glm::vec2(innerID % INNER_ROW, innerID / INNER_ROW) * smallImageSize;
	p->texOffset2 = outerOffset + glm::vec2(nextInnerID % INNER_ROW, nextInnerID / INNER_ROW) * smallImageSize;
	p->texCoordInfo = glm::vec2(smallImageSize, fmod(innerCount * lifeFactor, 1));
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

	auto& offsetToTexture = json["offsetToTexture"];
	_offsetToTexture = glm::vec2(offsetToTexture[0].get<float>(), offsetToTexture[1].get<float>());
	
	_drawObject->mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getQuad().get();
	_tempRotation = glm::mat4(1);
	_tempRotation *= glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1));
}

void ParticleComponent::registerUI() {
	ImGui::DragInt("ParticlesPerSecond", &_pps, 1.0);
}