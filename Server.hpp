#ifndef SERVER_HPP
# define SERVER_HPP

# include <cstdlib>
# include <iostream>
# include <vector>
# include <string>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <signal.h> 
# include <unistd.h>
# include <poll.h>
# include <deque>
# include <cstdlib>
# include <cstring>

# include "User.hpp"
# include "Channel.hpp"
# include "CmdManager.hpp"

# define VERSION "0.1"

class User;
class Channel;
class CmdManager;

class Server
{
private:
	int serverSocket;
	struct sockaddr_in serverAddress, clientAddr;
	static std::vector<struct pollfd> pollfds;
	std::deque<User> users;
	std::deque<Channel> channels;
	CmdManager cm;
	const std::string pass;
	char buffer[532];
	int port_;

	void handleClient(int clientSocket);

	Server();
	Server(const Server &server);
	Server &operator=(const Server &server);

public:
	Server(int port, const std::string _pass);
	~Server();

	void acceptConnections();
	static void endServer(const std::string &msg);
	static void signal_handler(int sig);
};

#endif
