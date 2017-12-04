#pragma once
#include <hydra/ext/api.hpp>

#include <SDL2/SDL_net.h>
#include <hydra/network/packets.hpp>

namespace Hydra::Network {
	class HYDRA_NETWORK_API TCPClient {
	public:
		TCPClient();
		~TCPClient();
		bool initialize(char* ip, int port);
		int send(void* data, int length);
		std::vector<Packet*> receiveData();
		bool isConnected();
		void close();
	private:
		SDLNet_SocketSet _sset;
		TCPsocket _tcp;
		bool _connected;
		std::vector<uint8_t> _data;
		int _waitingForData;
	};
}
