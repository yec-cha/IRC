#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <string>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <poll.h>

# include "User.hpp"
# include "Channel.hpp"

const int MAX_CONNECTIONS = 10;

class User;
class Channel;

class IRCServer {
private:
	int serverSocket;
	struct sockaddr_in serverAddress, clientAddr;
	std::vector<User> users;
	std::vector<Channel> channels;
	// struct pollfd pollfds[MAX_CONNECTIONS];
	std::vector<struct pollfd> pollfds;
	char buffer[532];
    int port_;

	void cmdNick(std::vector<User>::iterator &iter, std::string &msg);
	void beforeRegisterdMsg(std::string &cmd, std::string &msg, std::vector<User>::iterator &iter);
	void handleClient(int clientSocket);

public:
	IRCServer();
    IRCServer(int port);

	void acceptConnections();
};


#endif