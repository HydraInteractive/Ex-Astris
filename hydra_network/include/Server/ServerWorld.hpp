#pragma once
#include <hydra/world/world.hpp>
#include <NetPlayer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Server/ServerPlayer.hpp>
#include <Server/TCPHost.hpp>


class ServerWorld {
private:
	std::unique_ptr<Hydra::World::IWorld> _world;
	std::vector<ServerPlayer> _players;
public:
	HYDRA_API ServerWorld();
	HYDRA_API ~ServerWorld();

	HYDRA_API void initialize();
	HYDRA_API void sendPlayers(TCPHost* _conn, int64_t exception);
	HYDRA_API int getNewEntityID();
	HYDRA_API void addEntity();
	HYDRA_API ServerPlayer& addPlayer(const glm::vec3& pos, const glm::quat& rot, TCPsocket psocket);
	HYDRA_API void updateEntityTransform(TransformInfo pi, int64_t id);
	HYDRA_API void sendCurrentWorld(TCPHost* _conn);
	HYDRA_API void sendEntity(TCPHost* _conn, Hydra::World::IEntity* ent, int64_t exception = -1);
	HYDRA_API std::vector<ServerPlayer> getPlayers();

	HYDRA_API inline void fill(std::shared_ptr<Hydra::World::IEntity> ent, PacketSpawnEntityServer*& pse);
};
