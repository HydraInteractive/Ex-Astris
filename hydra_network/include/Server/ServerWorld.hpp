#pragma once
#include <hydra/world/world.hpp>
#include <../hydra_network/include/NetPlayer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <../hydra_network/include/Server/ServerPlayer.hpp>
#include <../hydra_network/include/Server/TCPHost.hpp>


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
	HYDRA_API int64_t addPlayer(const glm::vec3& pos, const glm::quat& rot, TCPsocket psocket);
	HYDRA_API void updateEntityTransform(TransformInfo pi, int64_t id);
	HYDRA_API void sendCurrentWorld(TCPHost* _conn);
};