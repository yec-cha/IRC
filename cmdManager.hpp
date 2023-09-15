#ifndef CMDMANAGER_HPP
#define CMDMANAGER_HPP

#include "User.hpp"
#include "Channel.hpp"
#include "ErrManager.hpp"
#include <iostream>
#include "color.hpp"
#include <vector>
#include <string>

class CmdManager
{
private:
	std::vector<User> &users;
	std::vector<Channel> &channels;

	void sendClient(int socket, const std::string msg);
public:
	CmdManager(std::vector<User> &_users, std::vector<Channel> &_channels);
	~CmdManager();

	void exeCmd(std::string msg, std::vector<User>::iterator &user);

	void cmd_NICK(std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void cmd_USER(std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void cmd_PASS(std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void beforeRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void afterRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
};

#endif