#pragma once

#include <hydra/world/world.hpp>
#include <hydra/network/packets.hpp>

namespace BarcodeServer {
	class Server;
	class Player;

	Hydra::Network::ServerDeletePacket* createServerDeletePacket(Hydra::World::EntityID entID);
	Hydra::Network::ServerPlayerPacket* createServerPlayerPacket(const std::string& name, const Hydra::Network::TransformInfo& ti);
	//Hydra::Network::ClientUpdatePacket* createClientUpdatePacket(Hydra::World::Entity* player);
	void createAndSendServerEntityPacket(Hydra::World::Entity* ent, Server* s);
	void createAndSendPlayerUpdateBulletPacket(Player* p, Server* s);
	void createAndSendPlayerShootPacket(Player* p, Hydra::Network::ClientShootPacket* csp, Server* s);

	void resolveClientUpdatePacket(Hydra::Network::ClientUpdatePacket* cup, Hydra::World::EntityID entityID);
	Hydra::World::Entity* resolveClientSpawnEntityPacket(Hydra::Network::ClientSpawnEntityPacket* csep, Hydra::World::EntityID entityID, Server* s);
	void resolveClientUpdateBulletPacket(Hydra::Network::ClientUpdateBulletPacket* cubp, nlohmann::json& dest);
	Hydra::World::Entity* resolveClientShootPacket(Hydra::Network::ClientShootPacket* csp, Player* p, Hydra::World::ISystem* bp);
}
