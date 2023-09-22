#ifndef CMDMANAGER_HPP
# define CMDMANAGER_HPP

# include <vector>
# include <string>
# include <deque>
# include <iostream>
# include <ctime>

# include "User.hpp"
# include "Channel.hpp"
# include "Messege.hpp"
# include "color.hpp"

class CmdManager
{
private:
	std::deque<User> &users;
	std::deque<Channel> &channels;
	const std::string &pass;

	CmdManager();
	static void sendClient(int socket, const std::string msg);

public:
	CmdManager(std::deque<User> &_users, std::deque<Channel> &_channels, const std::string &_pass);
	~CmdManager();

	void beforeRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void afterRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void exeCmd(std::string msg, std::deque<User>::iterator &user);
	void cmd_NICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_TOPIC(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_USER(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_PASS(std::vector<std::string> const &parameters, std::deque<User>::iterator &iter);
	void cmd_JOIN(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_PRIVMSG(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_MODE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_INVITE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void cmd_KICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser);
	void bot(const std::deque<Channel>::iterator &iter, const std::string &msg);
};

#endif