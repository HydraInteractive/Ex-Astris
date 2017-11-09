#pragma once
#include <Server.h>
#include <hydra/world/world.hpp>
#include <chrono>

struct Player {
	int serverid;
	EntityID entityid;
	bool connected;
	//..
	Player() {
		this->serverid = -1;
		this->entityid = -1;
		this->connected = false;
	}
};

class GameServer {
private:
	std::chrono::time_point<std::chrono::steady_clock> lastTime;
	float packetDelay;
	Server* _server;
	std::vector<EntityID> _networkEntities;
	std::vector<Player*> _players;
	void _sendWorld();
	void _convertEntityToTransform(ServerUpdatePacket::EntUpdate& dest, EntityID ent);
	void _updateWorld();
	bool _addPlayer(int id);
	void _resolvePackets(std::vector<Packet*> packets);
	int64_t _getEntityID(int serverid);
	void _setEntityID(int serverID, int64_t entityID);
	void _sendNewEntity(EntityID ent);
	void _deleteEntity(EntityID ent);
	void _handleDisconnects();
	Entity* _createEntity(std::string name, int parentID, bool serverSynced);

public:
	GameServer();
	~GameServer();
	bool initialize(int port);
	void run();
	void quit();
};