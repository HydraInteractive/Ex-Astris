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
	json["delay"] = delay;
	json["accumulator"] = accumulator;
	json["behaviour"] = static_cast<int>(behaviour);
	json["texture"] = static_cast<int>(texture);
	json["velocityX"] = tempVelocity[0];
	json["velocityY"] = tempVelocity[1];
	json["velocityZ"] = tempVelocity[2];
}

void ParticleComponent::deserialize(nlohmann::json & json){
	delay = json.value<float>("delay", 0);
	accumulator = json.value<int>("accumulator", 0);

	behaviour = static_cast<EmitterBehaviour>(json["behaviour"].get<int>());
	texture = static_cast<ParticleTexture>(json["texture"].get<int>());
	tempVelocity[0] = json.value<float>("velocityX", 0);
	tempVelocity[1] = json.value<float>("velocityY", 0);
	tempVelocity[2] = json.value<float>("velocityZ", 0);
}

void ParticleComponent::registerUI() {
	float pps = (int)(1.0f / delay);
	if (ImGui::DragFloat("Particles/Second", &pps)) {
		pps = std::max(0.0f, pps);
		delay = 1.0f / pps;
	}
	ImGui::InputFloat("Delay between particles", &delay, 0, 0, -1, ImGuiInputTextFlags_ReadOnly);
	ImGui::DragFloat("Accumulator", &accumulator, 1.0);

	ImGui::Combo("Emitter Behaviour", reinterpret_cast<int*>(&behaviour), EmitterBehaviourStr, static_cast<int>(EmitterBehaviour::MAX_COUNT));
	ImGui::Combo("Particle Texture", reinterpret_cast<int*>(&texture), ParticleTextureStr, static_cast<int>(ParticleTexture::MAX_COUNT));

	if (!ImGui::CollapsingHeader("Particles"))
		return;
	int i = -1;
	for (auto& p : particles) {
		i++;
		ImGui::PushID((void*)&p);
		ImGui::Text("Particle #%d", i);
		ImGui::SameLine();

		if (ImGui::Button("Edit"))
			ImGui::OpenPopup((std::string("particle") + std::to_string(i)).c_str());

		if (ImGui::BeginPopup((std::string("particle") + std::to_string(i)).c_str())) {
			ImGui::Text("Editing Particle #%d", i);

			p.transform.registerUI();

			ImGui::Text("Velocity");
			ImGui::DragFloat3("##velocity", glm::value_ptr(p.velocity));
			ImGui::Text("Acceleration");
			ImGui::DragFloat3("##acceleration", glm::value_ptr(p.acceleration));
			ImGui::Text("Life");
			ImGui::DragFloat("##life", &p.life);
			ImGui::Text("Start Life");
			ImGui::DragFloat("##startlife", &p.startLife);

			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
		ImGui::NextColumn();
		ImGui::PopID();
	}
}

void ParticleComponent::spawnParticles() {
	using world = Hydra::World::World;
	auto t = world::getEntity(entityID)->getComponent<Hydra::Component::TransformComponent>();

	auto findFree = [this]() -> Particle* {
		static size_t orgI = 0;
		size_t i = orgI;
		do {
			if (particles[i].life <= 0) {
				orgI = (i+1) % MaxParticleAmount;
				return &particles[i];
			}
			i = (i + 1) % MaxParticleAmount;
		} while (i != orgI);
		return nullptr;
	};
	switch (behaviour) {
	case EmitterBehaviour::PerSecond:
		while (accumulator >= delay) {
			Particle* p = findFree();
			if (!p)
				break;

			accumulator -= delay;
			const float velX = frand() * 2 - 2;
			const float velY = frand() * 6 - 1;
			const float velZ = frand() * 2 - 2;

			const float accX = frand() * 2 - 2;
			const float accY = frand() * 8.5f;
			const float accZ = frand() * 2 - 2;

			const float life = frand() + 1.f;

			const glm::vec3 vel = glm::normalize(glm::vec3(velX, velY, velZ));
			const glm::vec3 acc = glm::vec3(accX, accY, accZ);
			p->respawn(*t, vel, acc, life);
		}
		break;
	case EmitterBehaviour::Explosion:
		while (accumulator >= delay) {
			Particle* p = findFree();
			if (!p)
				break;

			accumulator -= delay;
			const float velX = (frand() * tempVelocity.x - 3.0f) + optionalNormal.x;
			const float velY = (frand() * tempVelocity.y - 2.0f) + optionalNormal.y;
			const float velZ = (frand() * tempVelocity.z - 3.0f) + optionalNormal.z;

			const float accX = velX * 2.0f;
			const float accY = velY * 2.0f;
			const float accZ = velZ * 2.0f;

			const float life = frand() * 4;
			const glm::vec3 vel = normalize(glm::vec3(velX, velY, velZ));
			const glm::vec3 acc = glm::vec3(accX, accY, accZ);
			p->respawn(*t, vel, acc, life);
		}
		break;
	case EmitterBehaviour::SpawnerBeam:
		while (accumulator >= delay) {
			Particle* p = findFree();
			if (!p)
				break;

			const float posX = frand() * 2.8f - 1.4f;
			const float posZ = frand() * 2.8f - 1.4f;

			t->position = glm::vec3(posX, 0, posZ);

			accumulator -= delay;
			const float velY = frand() * 4 - 1;
			const float accY = frand() * 3.5f;

			const float life = frand() + 1.f;

			const glm::vec3 vel = glm::normalize(glm::vec3(0, velY, 0));
			const glm::vec3 acc = glm::vec3(0, accY, 0);
			p->respawn(*t, vel, acc, life);
		}
		break;
	default:
		break;
	}
}
