#pragma once

#include <hydra/network/packets.hpp>

namespace Server {
	class Server;
	struct Player;

	Hydra::Network::ServerDeletePacket* createServerDeletePacket(EntityID entID);
	Hydra::Network::ServerPlayerPacket* createServerPlayerPacket(std::string name, TransformInfo ti);
	Hydra::Network::ClientUpdatePacket* createClientUpdatePacket(Entity* player);
	void createAndSendServerEntityPacket(Entity* ent, Server* s);
	void createAndSendPlayerUpdateBulletPacket(Player* p, Server* s);
	void createAndSendPlayerShootPacket(Player* p, Hydra::Network::ClientShootPacket* csp, Server* s);

	void resolveClientUpdatePacket(Hydra::Network::ClientUpdatePacket* cup, EntityID entityID);
	Entity* resolveClientSpawnEntityPacket(Hydra::Network::ClientSpawnEntityPacket* csep, EntityID entityID, Server* s);
	void resolveClientUpdateBulletPacket(Hydra::Network::ClientUpdateBulletPacket* cubp, nlohmann::json &dest);
	Entity* resolveClientShootPacket(Hydra::Network::ClientShootPacket* csp, Player* p, Hydra::System::BulletPhysicsSystem* bp);
}
