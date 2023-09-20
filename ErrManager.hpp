#include <iostream>
#include <string>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

#include "color.hpp"

class ErrManager
{
private:
	ErrManager(/* args */){};
	~ErrManager(){};

	static void sendClient(int socket, const std::string &msg)
	{
		send(socket, msg.c_str(), msg.length(), 0);
		std::cout << "server (to " << socket << ") :" << RED << msg << RESET << std::endl;
	};

public:
	static void send_409(int socket)
	{
		const std::string response = "409 :No origin specified\n";
		sendClient(socket, response);
	};

	static void send_431(int socket)
	{
		const std::string response = "431 :No nickname given\n";
		sendClient(socket, response);
	};

	static void send_432(int socket, const std::string &nick)
	{
		const std::string response = "432 " + nick + " :Erroneus nickname\n";
		sendClient(socket, response);
	};

	static void send_433(int socket, const std::string &nick, const std::string &newNick)
	{
		std::string response;
		if (nick == "")
			response = "433 anonymous " + newNick + " :Nickname is already in use\n";
		else
			response = "433 " + nick + " " + newNick + " :Nickname is already in use\n";
		sendClient(socket, response);
	};

	static void send_451(int socket)
	{
		const std::string response = "451 : client must be registered\n";
		sendClient(socket, response);
	};

	// ERR_NEEDMOREPARAMS
	static void send_461(int socket, const std::string &cmd)
	{
		const std::string response = "461 " + cmd + " :Not enough parameters\n";
		sendClient(socket, response);
	};

	static void send_462(int socket)
	{
		const std::string response = "462 :You may not reregister\n";
		sendClient(socket, response);
	};

	static void send_421(int socket, const std::string &cmd)
	{
		const std::string response = "421 " + cmd + " :Unknown command\n";
		sendClient(socket, response);
	};

	static void send_464(int socket)
	{
		const std::string response = "464 :Password incorrect\n";
		sendClient(socket, response);
	};
};
