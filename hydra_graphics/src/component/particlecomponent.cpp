#include <hydra/component/particlecomponent.hpp>
#include <hydra/engine.hpp>
#include <imgui/imgui.h>
#include <hydra/component/cameracomponent.hpp>
#include <algorithm>

#define frand() (float(rand()) / float(RAND_MAX))
#define ROW 8 // is always going to be a uniformly scaled texture.
using namespace Hydra::World;
using namespace Hydra::Component;

ParticleComponent::ParticleComponent(IEntity* entity) : IComponent(entity), _drawObject(entity->getDrawObject()), _pps(1), _behaviour(EmitterBehaviour::PerSecond), _accumulator(0.f){
	_drawObject->refCounter++;
	_drawObject->mesh = nullptr;
}

ParticleComponent::ParticleComponent(IEntity* entity, EmitterBehaviour behaviour, int nrOfParticles) : IComponent(entity), _drawObject(entity->getDrawObject()), 
_pps(nrOfParticles), _behaviour(behaviour), _accumulator(0.f){
	_drawObject->refCounter++;
	_drawObject->mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getQuad().get();
	_tempRotation = glm::mat4(1);
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
		//if (_particles.size() > 1)
		//	_sortParticles(_particles.size() - 1, 0);
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
	float dirX = frand() * 2 - 1;
	float dirY = frand() * 2 - 1;
	float dirZ = 0;
	p->spawn(glm::vec3(0), glm::normalize(glm::vec3(dirX, dirY, dirZ)) * glm::vec3(0.1), 2.f);
	_particles.push_back(p);
}

void ParticleComponent::_particlePhysics(float delta) {
	auto camera = Hydra::IEngine::getInstance()->getState()->getWorld()
		->getActiveComponents<CameraComponent>()[0]
		->getComponent<CameraComponent>();

	for (auto& p : _particles) {
		if (p->life <= p->elapsedTime)
			p->dead = true;

		if (p->dead)
			continue;

		if (camera)
			p->distanceToCamera = glm::distance(camera->getPosition(), p->pos);

		p->vel += p->vel * delta;
		p->pos += p->vel * delta;
		_updateTextureCoordInfo(p, delta);
		p->elapsedTime += delta;
		p->fixMX(_tempRotation);
	}
	_clearDeadParticles();
}

void ParticleComponent::_updateTextureCoordInfo(std::shared_ptr<Particle>& p, float delta) {
	float lifeFactor = p->elapsedTime / p->life;
	int stageCount = ROW*ROW;
	float atlasProg = lifeFactor * stageCount;
	int index1 = (int)floor(atlasProg);
	int index2 = index1 < stageCount - 1 ? index1 + 1 : index1;
	p->texCoordInfo = glm::vec2(ROW, fmod(atlasProg, 1));
	_setTextureOffset(p->texOffset1, index1);
	_setTextureOffset(p->texOffset2, index2);
}

void ParticleComponent::_setTextureOffset(glm::vec2& offset, int index) {
	int column = index % ROW;
	int row = index / ROW;
	offset.x = column / (float)ROW;
	offset.y = row / (float)ROW;
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

void ParticleComponent::_sortParticles(int left, int right) { // Quick sort
	int i = left, j = right;
	std::shared_ptr<Particle> tmp;
	float pivot = _particles[(left + right) / 2]->distanceToCamera;
	while (i <= j) {
		while (_particles[i]->distanceToCamera < pivot)
			i++;
		while (_particles[j]->distanceToCamera > pivot)
			j--;
		if (i >= j) {
			tmp = _particles[i];
			_particles[i] = _particles[j];
			_particles[j] = tmp;
			i++;
			j--;
		}
	}
	if (left < j)
		_sortParticles(left, j);
	if (i < right)
		_sortParticles(i, right);
}

void ParticleComponent::serialize(nlohmann::json & json) const{
	json = {
		{ "pps", _pps},
		{ "accumulator", _accumulator},
		{ "behaviour", (int)_behaviour}
	};
}

void ParticleComponent::deserialize(nlohmann::json & json){
	auto& pps = json["pps"];
	 _pps = pps.get<int>();

	auto& accumulator = json["accumulator"];
	_accumulator = accumulator.get<int>();

	auto& behaviour = json["behaviour"];
	_behaviour = (EmitterBehaviour)behaviour.get<int>();
	
	_drawObject->mesh = Hydra::IEngine::getInstance()->getState()->getMeshLoader()->getQuad().get();
	_tempRotation = glm::mat4(1);
	_tempRotation *= glm::angleAxis(glm::radians(90.f), glm::vec3(0, 0, 1));
}

void ParticleComponent::registerUI(){
	ImGui::DragInt("ParticlesPerSecond", &_pps, 1.0);
}
