#ifndef CMDMANAGER_HPP
# define CMDMANAGER_HPP

# include "User.hpp"
# include "Channel.hpp"
# include "ErrManager.hpp"
# include <iostream>
# include "color.hpp"
# include <vector>
# include <string>
# include <deque>

class CmdManager
{
private:
	std::deque<User> &users;
	std::deque<Channel> &channels;
	const std::string &pass;

	static void sendClient(int socket, const std::string msg);

public:
	CmdManager(std::deque<User> &_users, std::deque<Channel> &_channels, const std::string &_pass);
	~CmdManager();

	void exeCmd(std::string msg, std::deque<User>::iterator &user);

	void cmd_NICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_TOPIC(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_USER(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_PASS(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void beforeRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void afterRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_JOIN(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_PRIVMSG(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_MODE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_INVITE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_KICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	

	// RPL_CHANNELMODEIS (324) "<client> <channel> <modestring> <mode arguments>..."
	static void RPL_CHANNELMODEIS_324(int socket, const std::string &client, const std::string &channel)
	{
		const std::string response = "324 " + client + " " + channel + "\n";
		sendClient(socket, response);
	};

	// static void RPL_CHANNELMODEIS_324(int socket, const std::string &client, const std::string &channel, const std::string &modestring)
	// {
	// 	const std::string response = "324 " + client + " " + channel + " " + modestring + "\n";
	// 	sendClient(socket, response);
	// };

	// static void RPL_CHANNELMODEIS_324(int socket, const std::string &client, const std::string &channel, const std::string &modestring, const std::string &modearguments)
	// {
	// 	const std::string response = "324 " + client + " " + channel + " " + modestring + " " + modearguments + "\n";
	// 	sendClient(socket, response);
	// };


	static void send_332(int socket, const std::string &channelName, const std::string &topic)
	{
		const std::string response = "332 " + channelName + " :" + topic + "\n";
		sendClient(socket, response);
	};

	static void send_331(int socket, const std::string &channelName)
	{
		const std::string response = "331 " + channelName + " :No topic is set\n";
		sendClient(socket, response);
	};
};

#endif