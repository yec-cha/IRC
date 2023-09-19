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
	const std::string &pass;

	void sendClient(int socket, const std::string msg);

public:
	CmdManager(std::vector<User> &_users, std::vector<Channel> &_channels, const std::string &_pass);
	~CmdManager();

	void exeCmd(std::string msg, std::vector<User>::iterator &user);

	void cmd_NICK(std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void cmd_USER(std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void cmd_PASS(std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void beforeRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);
	void afterRegisteredMsg(std::string &cmd, std::vector<std::string> const &parameters, std::vector<User>::iterator &iter);

	void printInfo(std::vector<std::string> const &parameters, std::vector<User>::iterator &iterUser) const
	{
		std::cout << "[INFO]" << std::endl;
		std::cout << "USER NAME: " << iterUser->getUserName() << std::endl;
		std::cout << "USER NICK: " << iterUser->getNickName() << std::endl;

		std::cout << " - PARAMS" << std::endl;
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			std::cout << "idx: " << i << ", CONTENT: " << parameters[i] << std::endl;
		}
		std::cout << " -" << std::endl;
		std::cout << "[INFO]" << std::endl;
	}

	void cmd_JOIN(std::vector<std::string> const &parameters, std::vector<User>::iterator &iterUser)
	{
		std::vector<Channel>::iterator iterChannel = this->channels.begin();
		// printInfo(parameters, iterUser);
		for (; iterChannel < this->channels.end(); ++iterChannel)
		{
			if (iterChannel->getName() == parameters[0])
			{
				// std::cout << "params: " << parameters[0] << std::endl;
				// std::cout << "iter: " << iterChannel->getName() << std::endl;
				iterChannel->addUser(*iterUser);
				// std::cout << "already exist" << std::endl;
			}
		}
		if (iterChannel == this->channels.end())
		{
			Channel newChannel(parameters[0], *iterUser);
			this->channels.push_back(newChannel);
			// std::vector<Channel>::iterator test = this->channels.begin();
			// test->printInfo();
			// std::cout << "make new one" << std::endl;
		}
		std::vector<Channel>::iterator test = this->channels.begin();
		test->welcomeChannel(iterUser);
		// std::cout << "make new one" << std::endl;
		// std::cout << "[TEST 2]channel list -------" << std::endl;
		// for (std::vector<Channel>::iterator test = this->channels.begin(); test < this->channels.end(); ++test) {
		// 	std::cout << "channel: " << test->getName() << std::endl;
		// }
		// std::cout << "[TEST 2]channel list -------" << std::endl;
	}

	void cmd_PRIVMSG(std::vector<std::string> const &parameters, std::vector<User>::iterator &iterUser)
	{
		// (void)iterUser;
		// std::cout << "[TEST 1] PRIVMSG -------------------" << std::endl;
		// for (size_t i = 0; i < parameters.size(); ++i)
		// 	std::cout << "params[" << i << "]: " << parameters[i] << std::endl;
		// std::cout << "------------------------------------" << std::endl;
		std::vector<Channel>::iterator iterChannel = this->channels.begin();
		for (; iterChannel < this->channels.end(); ++iterChannel)
		{
			if (iterChannel->getName() == parameters[0])
			{
				iterChannel->sendPRIVMSG(parameters, iterUser);
				// std::map<int, User&>::iterator it = iterChannel->getFirst();
				// std::map<int, User&>::iterator end = iterChannel->getLast();
				// for (; it < end; ++it) {
				// 	if (iterChannel[i].)
				// 	// send(iter->getSocket(), response.c_str(), response.length(), 0);
				// }
			}
		}
		// std::cout << "[TEST 1] PRIVMSG -------------------" << std::endl;
	}
};

#endif