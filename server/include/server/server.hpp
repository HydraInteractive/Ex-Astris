#pragma once
#include <SDL2/SDL_net.h>
#include <vector>
#include <server/clienthandler.hpp>

//This class should (only) set up TCP connections with clients aswell as send and receive data from clients.

namespace BarcodeServer {
	class Server {
	public:
		Server();
		~Server();

		///<summary>
		///Starts up a server on said port and returns true if successfull.
		///</summary>
		bool initialize(int port);

		///<summary>
		///Checks for incoming connections and returns client id on new connection, returns -1 otherwise.
		///</summary>
		int checkForNewClients();

		///<summary>
		///Sends data to every connected client.
		///</summary>
		void sendDataToAll(char* data, int length);

		///<summary>
		///Sends data to specific connected client. Returns number of bytes sent.
		///</summary>
		int sendDataToClient(char* data, int length, int clientID);

		///<summary>
		///Sends data to every connected client except one specified client.
		///</summary>
		void sendDataToAllExcept(char* data, int length, int clientID);

		std::vector<int> getDisconnects();

		///<summary>
		///Delete packets after use.
		///</summary>
		std::vector<Hydra::Network::Packet*> receiveData();

		bool isRunning();

	private:
		ClientHandler _clientHandler;

		TCPsocket _sock;
		bool _running;
		int _port;
	};
}
