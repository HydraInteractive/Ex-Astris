#include <hydra/component/particlecomponent.hpp>
#include <hydra/engine.hpp>
#include <imgui/imgui.h>
#include <hydra/component/cameracomponent.hpp>
#include <algorithm>

#define frand() (float(rand()) / float(RAND_MAX))

using namespace Hydra::World;
using namespace Hydra::Component;

ParticleComponent::~ParticleComponent() {}

void ParticleComponent::serialize(nlohmann::json & json) const{
	json = {
		{ "delay", delay},
		{ "accumulator", accumulator},
		{ "emitterPos", { emitterPos.x, emitterPos.y, emitterPos.z } },
		{ "behaviour", static_cast<int>(behaviour)},
		{ "texture", static_cast<int>(texture)}
	};
}

void ParticleComponent::deserialize(nlohmann::json & json){
	delay = json["delay"].get<float>();
	accumulator = json["accumulator"].get<int>();

	auto& pos = json["emitterPos"];
	emitterPos = glm::vec3{ pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };

	behaviour = static_cast<EmitterBehaviour>(json["behaviour"].get<int>());
	texture = static_cast<ParticleTexture>(json["texture"].get<int>());
}

void ParticleComponent::registerUI() {
	float pps = (int)(1.0f / delay);
	if (ImGui::DragFloat("Particles/Second", &pps)) {
		pps = std::max(0.0f, pps);
		delay = 1.0f / pps;
	}
	ImGui::InputFloat("Delay between particles", &delay, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);
	ImGui::DragFloat("Accumulator", &accumulator, 1.0);
	ImGui::DragFloat3("Emitter Position", glm::value_ptr(emitterPos), 0.1f);

	ImGui::Combo("Emitter Behaviour", reinterpret_cast<int*>(&behaviour), EmitterBehaviourStr, static_cast<int>(EmitterBehaviour::MAX_COUNT));
	ImGui::Combo("Particle Texture", reinterpret_cast<int*>(&texture), ParticleTextureStr, static_cast<int>(ParticleTexture::MAX_COUNT));
}

void ParticleComponent::spawnParticles() {
	switch (behaviour) {
	case EmitterBehaviour::PerSecond:
		while (accumulator >= delay) {
			accumulator -= delay;
			std::shared_ptr<Particle> p = std::make_shared<Particle>();
			float dirX = frand() * 2 - 2;
			float dirY = frand() * 6 - 1;
			float dirZ = 0;
			glm::vec3 a = glm::vec3(frand() * 2 - 2, frand() * 8.5f, 0);
			p->spawn(emitterPos, glm::normalize(glm::vec3(dirX, dirY, dirZ)), a, frand() * 1.f + 1.f);
			particles.push_back(p);
		}
		break;
	default:
		break;
	}
}
