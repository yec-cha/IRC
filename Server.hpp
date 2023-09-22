#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h> 
#include <unistd.h>
#include <poll.h>
#include <deque>
#include "User.hpp"
#include "Channel.hpp"
#include "CmdManager.hpp"

#define VERSION "0.1"
const int MAX_CONNECTIONS = 10;

class User;
class Channel;
class CmdManager;

class IRCServer
{
private:
	int serverSocket;
	struct sockaddr_in serverAddress, clientAddr;
	std::deque<User> users;
	std::deque<Channel> channels;
	// struct pollfd pollfds[MAX_CONNECTIONS];
	static std::vector<struct pollfd> pollfds;

	CmdManager cm;

	char buffer[532];
	int port_;

	const std::string pass;

	void handleClient(int clientSocket);

public:
	IRCServer();
	IRCServer(int port, const std::string _pass);

	static void endServer(const std::string &msg)
	{
		for (size_t i = 1; i < pollfds.size(); i++)
		{
			send(pollfds[i].fd, msg.c_str(), msg.size(), 0);
			close(pollfds[i].fd);
		}
		pollfds.clear();
	}

	static void signal_handler(int sig)
	{
		if (sig == SIGQUIT || sig == SIGINT)
		{
			std::string quitMsg = ":ft_IRC QUIT :server end now";
	
			endServer(quitMsg);

			std::cout << BOLDMAGENTA << "\n\nSERVER END\n"<< RESET << std::endl;
			exit(0);
		}
	}
	void acceptConnections();
};

#endif
