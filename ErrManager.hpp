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

class ErrManager
{
private:
	ErrManager(/* args */){};
	~ErrManager(){};

public:
	static void send_431(int socket)
	{
		const std::string response = "431 :No nickname given\n";
		send(socket, response.c_str(), response.length(), 0);
	};

	static void send_432(int socket, const std::string &nick)
	{
		const std::string response = "432 " + nick + " :Erroneus nickname\n";
		send(socket, response.c_str(), response.length(), 0);
	};

	static void send_433(int socket, const std::string &nick)
	{
		const std::string response = "433 " + nick + " :Nickname is already in use\n";
		send(socket, response.c_str(), response.length(), 0);
	};

	static void send_461(int socket, const std::string &cmd)
	{
		const std::string response = "461 " + cmd + " :Not enough parameters\n";
		send(socket, response.c_str(), response.length(), 0);
	};

	static void send_462(int socket)
	{
		const std::string response = "462 :You may not reregister\n";
		send(socket, response.c_str(), response.length(), 0);
	};
};
