#pragma once
#include <Server.h>
#include <hydra/world/world.hpp>
#include <chrono>

struct Player {
	int serverid;
	std::shared_ptr<Hydra::World::IEntity> entityptr;
	int64_t entityid;
	bool entityAlive;
	bool connected;
	//..
	Player() {
		this->serverid = -1;
		this->entityid = -1;
		this->entityAlive = false;
		this->connected = false;
	}
};

class GameServer {
private:
	std::chrono::time_point<std::chrono::steady_clock> lastTime;
	float packetDelay;
	Server* _server;
	std::unique_ptr<Hydra::World::IWorld> _world;
	std::vector<std::shared_ptr<Hydra::World::IEntity>> _networkEntities;
	std::vector<Player*> _players;
	void _sendWorld();
	void _convertEntityToTransform(ServerUpdatePacket::EntUpdate& dest, std::shared_ptr<Hydra::World::IEntity> ent);
	void _updateWorld();
	bool _addPlayer(int id);
	void _resolvePackets(std::vector<Packet*> packets);
	int64_t _getEntityID(int serverid);
	void _setEntityID(int serverID, int64_t entityID);
	void _sendNewEntity(Hydra::World::IEntity* ent);
	void _deleteEntity(Hydra::World::IEntity* ent);
	void _handleDisconnects();
	Hydra::World::IEntity* _createEntity(std::string name, int parentID, bool serverSynced);

public:
	GameServer();
	~GameServer();
	bool initialize(int port);
	void run();
	void quit();
};