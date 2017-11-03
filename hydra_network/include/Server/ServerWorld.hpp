#pragma once
#include <hydra/world/world.hpp>
#include <NetPlayer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Server/ServerPlayer.hpp>
#include <Server/TCPHost.hpp>


class ServerWorld {
private:
	std::vector<ServerPlayer> _players;
public:
	HYDRA_NETWORK_API ServerWorld();
	HYDRA_NETWORK_API ~ServerWorld();

	HYDRA_NETWORK_API void initialize();
	HYDRA_NETWORK_API void sendPlayers(TCPHost* _conn, int64_t exception);
	HYDRA_NETWORK_API int getNewEntityID();
	HYDRA_NETWORK_API void addEntity();
	HYDRA_NETWORK_API ServerPlayer& addPlayer(const glm::vec3& pos, const glm::quat& rot, TCPsocket psocket);
	HYDRA_NETWORK_API void updateEntityTransform(TransformInfo pi, int64_t id);
	HYDRA_NETWORK_API void sendCurrentWorld(TCPHost* _conn);
	HYDRA_NETWORK_API void sendEntity(TCPHost* _conn, Hydra::World::Entity* ent, int64_t exception = -1);
	HYDRA_NETWORK_API std::vector<ServerPlayer> getPlayers();

	HYDRA_NETWORK_API inline void fill(std::shared_ptr<Hydra::World::Entity> ent, PacketSpawnEntityServer*& pse);
};
