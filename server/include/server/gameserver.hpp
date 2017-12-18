#pragma once
#include <server/server.hpp>
#include <server/tilegeneration.hpp>
#include <hydra/world/world.hpp>
#include <chrono>
#include <hydra/system/deadsystem.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/system/aisystem.hpp>
#include <hydra/system/bulletsystem.hpp>
#include <hydra/system/spawnersystem.hpp>
#include <hydra/system/perksystem.hpp>
#include <hydra/system/lifesystem.hpp>
#include <hydra/system/pickupsystem.hpp>
#include <json.hpp>

namespace BarcodeServer {
	struct Player {
		int serverid = -1;
		EntityID entityid = -1;
		bool connected = false;
		nlohmann::json bullet;
		float shootAnimation = 0;

		Player() {}
	};

	class GameServer {
	public:
		GameServer();
		~GameServer();
		bool initialize(int port);
		void start();
		void run();
		void quit();
		Hydra::System::BulletPhysicsSystem _physicsSystem;
		inline BarcodeServer::Server* getServer() { return this->_server; }
		void syncEntity(Hydra::World::Entity* entity);
		void deleteEntity(Hydra::World::EntityID ent);
	private:

		std::chrono::time_point<std::chrono::high_resolution_clock> _lastTime;
		float _packetDelay;
		Server* _server = nullptr;
		std::vector<Hydra::World::EntityID> _networkEntities;
		std::vector<Player*> _players;
		std::unique_ptr<TileGeneration> _tileGeneration;
		bool** _pathfindingMap = nullptr;
		std::string _pvsData;
		size_t level = 1;
		struct SyncBoi
		{
			Hydra::Network::ServerID client, ai;
		};
		std::vector<SyncBoi> aiInspectorSync;
		Hydra::World::EntityID _bossID;

		Hydra::System::DeadSystem _deadSystem;
		Hydra::System::AISystem _aiSystem;
		Hydra::System::BulletSystem _bulletSystem;
		Hydra::System::SpawnerSystem _spawnerSystem;
		Hydra::System::PerkSystem _perkSystem;
		Hydra::System::LifeSystem _lifeSystem;
		Hydra::System::PickUpSystem _pickupSystem;

		void _makeWorld();
		void _spawnBoss();
		void _sendWorld();
		void _convertEntityToTransform(Hydra::Network::ServerUpdatePacket::EntUpdate& dest, Hydra::World::EntityID ent);
		void _resolvePackets(std::vector<Hydra::Network::Packet*> packets);
		int64_t _getEntityID(int serverid);
		void _setEntityID(int serverID, int64_t entityID);
		void _handleDisconnects();
		Hydra::World::Entity* _createEntity(const std::string& name, Hydra::World::EntityID parentID, bool serverSynced);
		Player* _getPlayer(Hydra::World::EntityID id);
		bool _addPlayer(int id);
		void _sendPathInfo();

		static void _onRobotShoot(WeaponComponent& weapon, Entity* bullet, void* userdata);
		void _resolveClientRequestAIInfoPacket(Hydra::Network::ClientRequestAIInfoPacket* packet);
	};
}
