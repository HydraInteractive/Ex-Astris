#include <TCPConnection.hpp>
#include <hydra/engine.hpp>
#include <TCPClient.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>




//if (SDLNet_ResolveHost(&ip, NULL, 3160) == -1) {
//	printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
//	exit(1);
//}
//socket = SDLNet_TCP_Open(&ip);
//if (!socket) {
//	printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
//	exit(2);
//}



TCPClient::TCPClient() {
}

TCPClient::~TCPClient() {
	delete[] _msg;
}

bool TCPClient::initiate(IPaddress ip, const char* s) {
	_msg = new char[NETWORK_MAX_LENGTH];
	this->_ip = ip;
	if (SDLNet_ResolveHost(&_ip, s, ip.port) == -1) {
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return false;
	}

	this->_socket = SDLNet_TCP_Open(&_ip);
	if (!_socket) {
		std::string str = SDLNet_GetError();
		printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		return false;
	}
	this->_ss = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(this->_ss, this->_socket);
	return true;
}

void TCPClient::close() {
}

 void TCPClient::update() { // TODO
	 this->receivePacket();
}

 HYDRA_API void TCPClient::sendPacket(char * data, int length) {
	if (this->_socket) {
		 SDLNet_TCP_Send(this->_socket, data, length);
	}
 }

 std::vector<NetPacket*> TCPClient::receivePacket() {
	int result;
	std::vector<NetPacket*> packets;
	if (SDLNet_CheckSockets(this->_ss, 0) <= 0)
		return packets;
	//if (SDLNet_CheckSockets(this->_ss, 0) == -1) {
	//	return nullptr;
	//}

	result = SDLNet_TCP_Recv(_socket, _msg, NETWORK_MAX_LENGTH);
	int max = result;
	while(result > 0) {
		NetPacket* np = (NetPacket*)((char*)(&_msg[max - result]));
		switch (np->header.type) {
		case PacketType::HelloWorld:
			result -= sizeof(PacketHelloWorld);
			packets.push_back((PacketHelloWorld*)np);
			break;
		case PacketType::ChangeID: //TODO
			result -= sizeof(PacketChangeID);
			packets.push_back((PacketChangeID*)np);
			break;
		case PacketType::ClientUpdate: //TODO
			result -= sizeof(PacketClientUpdate);
			packets.push_back((PacketClientUpdate*)np);
			break;
		case PacketType::SpawnEntityClient: //TODO
			result -= sizeof(PacketSpawnEntityClient);
			packets.push_back((PacketSpawnEntityClient*)np);
			break;
		case PacketType::SpawnEntityServer:
			result -= (((PacketSpawnEntityServer*)np)->packetLength());
			packets.push_back((PacketSpawnEntityServer*)np);
			break;
		case PacketType::ServerUpdate: 
			result -= ((PacketServerUpdate*)np)->getPacketSize();
			packets.push_back((PacketServerUpdate*)np);
			break;

		default:
			//Hydra::IEngine::getInstance()->log(Hydra::LogLevel::warning, "Network Error: Packet(s) not recognized.");
			result = 0;
			break;
		}
	}
	return packets;
}


