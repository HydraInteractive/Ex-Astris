#pragma once
#include <server/server.hpp>
#include <hydra/world/world.hpp>
#include <chrono>
#include <hydra/system/bulletphysicssystem.hpp>
#include <hydra/system/aisystem.hpp>
#include <hydra/system/bulletsystem.hpp>
#include <hydra/system/spawnersystem.hpp>
#include <hydra/system/perksystem.hpp>
#include <hydra/system/lifesystem.hpp>
#include <json.hpp>

struct Player {
	int serverid;
	EntityID entityid;
	bool connected;
	nlohmann::json bullet;
	//..
	Player() {
		this->serverid = -1;
		this->entityid = -1;
		this->connected = false;
	}
};

class GameServer {
private:
	//DEFAULT SHIT

	//DEFAULT SHIT

	std::chrono::time_point<std::chrono::steady_clock> lastTime;
	float packetDelay;
	Server* _server;
	std::vector<EntityID> _networkEntities;
	std::vector<Player*> _players;


	Hydra::System::BulletPhysicsSystem* _physicsSystem;
	Hydra::System::AISystem* _aiSystem;
	Hydra::System::BulletSystem* _bulletSystem;
	Hydra::System::SpawnerSystem* _spawnerSystem;
	Hydra::System::PerkSystem* _perkSystem;
	Hydra::System::LifeSystem* _lifeSystem;
	//Hydra::System::PlayerSystem _playerSystem;
	//Hydra::System::RendererSystem _rendererSystem;
	//Hydra::System::SoundFxSystem _soundFxSystem;
	//Hydra::System::CameraSystem _cameraSystem;
	//Hydra::System::LightSystem _lightSystem;
	//Hydra::System::ParticleSystem _particleSystem;
	//Hydra::System::AbilitySystem _abilitySystem;

	void _sendWorld();
	void _convertEntityToTransform(ServerUpdatePacket::EntUpdate& dest, EntityID ent);
	void _updateWorld();
	bool _addPlayer(int id);
	void _resolvePackets(std::vector<Packet*> packets);
	int64_t _getEntityID(int serverid);
	void _setEntityID(int serverID, int64_t entityID);
	void _sendNewEntity(EntityID ent, int serverIDexception);
	void _deleteEntity(EntityID ent);
	void _handleDisconnects();
	Entity* _createEntity(std::string name, EntityID parentID, bool serverSynced);
	Player* getPlayer(EntityID id);
public:
	GameServer();
	~GameServer();
	bool initialize(int port);
	void start();
	void run();
	void quit();
};
