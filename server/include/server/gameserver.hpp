#pragma once
#include <server/server.hpp>
#include <hydra/world/world.hpp>
#include <chrono>
#include <hydra/system/deadsystem.hpp>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/system/aisystem.hpp>
#include <hydra/system/bulletsystem.hpp>
#include <hydra/system/spawnersystem.hpp>
#include <hydra/system/perksystem.hpp>
#include <hydra/system/lifesystem.hpp>
#include <json.hpp>

namespace BarcodeServer {

	struct Player {
		int serverid = -1;
		EntityID entityid = -1;
		bool connected = false;
		nlohmann::json bullet;

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

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
		float packetDelay;
		Server* _server = nullptr;
		std::vector<Hydra::World::EntityID> _networkEntities;
		std::vector<Player*> _players;

		Hydra::System::DeadSystem _deadSystem;
		Hydra::System::BulletPhysicsSystem _physicsSystem;
		Hydra::System::AISystem _aiSystem;
		Hydra::System::BulletSystem _bulletSystem;
		Hydra::System::SpawnerSystem _spawnerSystem;
		Hydra::System::PerkSystem _perkSystem;
		Hydra::System::LifeSystem _lifeSystem;
		//Hydra::System::PlayerSystem _playerSystem;
		//Hydra::System::RendererSystem _rendererSystem;
		//Hydra::System::SoundFxSystem _soundFxSystem;
		//Hydra::System::CameraSystem _cameraSystem;
		//Hydra::System::LightSystem _lightSystem;
		//Hydra::System::ParticleSystem _particleSystem;
		//Hydra::System::AbilitySystem _abilitySystem;

		void _sendWorld();
		void _convertEntityToTransform(Hydra::Network::ServerUpdatePacket::EntUpdate& dest, Hydra::World::EntityID ent);
		void _updateWorld();
		bool _addPlayer(int id);
		void _resolvePackets(std::vector<Hydra::Network::Packet*> packets);
		int64_t _getEntityID(int serverid);
		void _setEntityID(int serverID, int64_t entityID);
		void _sendNewEntity(Hydra::World::EntityID ent, int serverIDexception);
		void _deleteEntity(Hydra::World::EntityID ent);
		void _handleDisconnects();
		Entity* _createEntity(const std::string& name, Hydra::World::EntityID parentID, bool serverSynced);
		Player* getPlayer(Hydra::World::EntityID id);

	};

}
