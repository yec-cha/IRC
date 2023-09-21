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
	// ERR_CHANNELISFULL (403) "<client> <channel> :No such channel"
	static void send_403(int socket, const std::string &client, const std::string &channel)
	{
		const std::string response = "403 " + client + " " + channel + " :No such channel\n";
		sendClient(socket, response);
	};

	// ERR_NOTONCHANNEL (442) "<client> <channel> :You're not on that channel"
	static void send_442(int socket, const std::string &client, const std::string &channel)
	{
		const std::string response = "442 " + client + " " + channel + " :You're not on that channel\n";
		sendClient(socket, response);
	};

	// ERR_NEEDMOREPARAMS (461) "<client> <command> :Not enough parameters"
	static void send_461(int socket, const std::string &client, const std::string &command)
	{
		const std::string response = "461 " + client + " " + command + " :Not enough parameters\n";
		sendClient(socket, response);
	};

	// ERR_CHANOPRIVSNEEDED (482) <client> <channel> :You're not channel operator"
	static void send_482(int socket, const std::string &client, const std::string &channel)
	{
		const std::string response = "482 " + client + " " + channel + " :You're not channel operator\n";
		sendClient(socket, response);
	};

	// ERR_USERONCHANNEL (443) "<client> <nick> <channel> :is already on channel"
	static void send_443(int socket, const std::string &client, const std::string& user, const std::string &channelName)
	{
		const std::string response = "443 " + client + " " + user + " " + channelName + " :is already on channel\n";
		sendClient(socket, response);
	};
	

	// // ERR_NOSUCHCHANNEL (403) 
  	// // "<client> <channel> :No such channel"
	// static void send_403(int socket, const std::string& channelName)
	// {
	// 	const std::string response = "409 :No origin specified\n";
	// 	sendClient(socket, response);
	// };

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

	// ERR_BADCHANNELKEY
	static void send_475(int socket, const std::string &nickName, const std::string &channelName)
	{
		const std::string response = "475 " + nickName + " " + channelName + " :Cannot join channel (+k)\n";
		sendClient(socket, response);
	};

	// ERR_INVITEONLYCHAN
	static void send_473(int socket, const std::string &nickName, const std::string &channelName)
	{
		const std::string response = "473 " + nickName + " " + channelName + " :Cannot join channel (+i)\n";
		sendClient(socket, response);
	};

	// ERR_CHANNELISFULL
	static void send_471(int socket, const std::string &nickName, const std::string &channelName)
	{
		const std::string response = "471 " + nickName + " " + channelName + " :Cannot join channel (+l)\n";
		sendClient(socket, response);
	};
	
	
	

	
};
