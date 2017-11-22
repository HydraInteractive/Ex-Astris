#pragma once
#include <vector>
#include <SDL2/SDL_net.h>
#include <server/packets.hpp>

#define MAX_NETWORK_LENGTH 100000

namespace Server {
	class ClientHandler {
	private:
		struct Client {
			bool isDead;
			TCPsocket socket;
			int id;
			int socketSet;
		};

		struct SocketSet {
			SDLNet_SocketSet set;
			int nrOfClients;

			SocketSet(SDLNet_SocketSet s) {
				this->set = s;
				this->nrOfClients = 0;
			}
		};

		std::vector<SocketSet> _sets;
		std::vector<Client*> _clients;
		std::vector<int> _disconnectedClients;
		char* _msg;
		int _currID;

		int _generateClientID();
	public:
		ClientHandler();
		~ClientHandler();

		void initialize(char* msg);
		TCPsocket getSocketFromID(int id);
		///<summary>
		///Returns the number of clients that have pending packets.
		///</summary>
		int getActivity();

		///<summary>
		///Delete packets after use!
		///</summary>
		std::vector<Packet*> getReceivedData(int pending);

		///<summary>
		///Returns id of new client. Quite inefficient, However shouldn't be a problem as this function is rarely called. Also, Dan sucks. 
		///</summary>
		int addNewConnection(TCPsocket sock);
		int sendData(char* data, int len, int clientID);

		std::vector<int> getAllClients();

		std::vector<int> getDisconnectedClients();

		void disconnectClient(int id);

		int getNrOfClients();
	};
}
