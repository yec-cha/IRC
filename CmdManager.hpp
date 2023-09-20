#ifndef CMDMANAGER_HPP
#define CMDMANAGER_HPP

#include "User.hpp"
#include "Channel.hpp"
#include "ErrManager.hpp"
#include <iostream>
#include "color.hpp"
#include <vector>
#include <string>
#include <deque>

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

	void printInfo(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser) const
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

	void cmd_JOIN(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
	{
		if (parameters.size() < 1)
		{
			ErrManager::send_461(iterUser->getSocket(), "JOIN");
			return;
		}

		std::map<std::string, std::string> channelList;

		std::vector<std::string> channelName;
		std::vector<std::string> key;

		std::istringstream ssName(parameters[0]);
		std::string token;

		while (std::getline(ssName, token, ','))
		{
			if (!token.empty())
				channelName.push_back(token);
		}

		if (parameters.size() >= 2)
		{
			std::istringstream ssKey(parameters[1]);
			std::string token2;
			while (std::getline(ssKey, token2, ','))
			{
				if (!token2.empty())
					key.push_back(token);
			}
		}

		std::pair<std::string, std::string> tmp;
		for (size_t i = 0; i < channelName.size(); i++)
		{
			tmp.first = channelName[i];
			if (i < key.size())
				tmp.second = key[i];
			else
				tmp.second = "";
			channelList.insert(tmp);
		}

		std::deque<Channel>::iterator iterChannel = channels.begin();

		for (std::map<std::string, std::string>::iterator it = channelList.begin(); it != channelList.end(); it++)
		{
			for (iterChannel = channels.begin(); iterChannel < this->channels.end(); ++iterChannel)
			{
				if (iterChannel->getName() == it->first)
				{
					if (iterChannel->getType() == 0)
						iterChannel->addUser(*iterUser);
					if (iterChannel->getType() == 1)
					{
						if (iterChannel->getPassword() == it->second)
							iterChannel->addUser(*iterUser);
						else
						{
							// bad password
							break;
						}
					}
					if (iterChannel->getType() == 2)
					{
						// invite only msg
						break;
					}
					break;
				}
			}
			if (iterChannel == this->channels.end())
			{

				Channel newChannel(parameters[0], *iterUser);
				this->channels.push_back(newChannel);

				iterChannel = channels.end() - 1;
			}
		}

		// std::cout << "make new one" << std::endl;
		// std::cout << "[TEST 2]channel list -------" << std::endl;
		// for (std::deque<Channel>::iterator test = this->channels.begin(); test < this->channels.end(); ++test) {
		// 	std::cout << "channel: " << test->getName() << std::endl;
		// }
		// std::cout << "[TEST 2]channel list -------" << std::endl;
	}

	void cmd_PRIVMSG(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
	{
		// (void)iterUser;
		// std::cout << "[TEST 1] PRIVMSG -------------------" << std::endl;
		// for (size_t i = 0; i < parameters.size(); ++i)
		// 	std::cout << "params[" << i << "]: " << parameters[i] << std::endl;
		// std::cout << "------------------------------------" << std::endl;
		std::deque<Channel>::iterator iterChannel = this->channels.begin();
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

	void cmd_MODE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
	{
		// MODE AA +i
		// MODE #EE

		// ex) /mode #4444 +i a
		// MODE #4444 +ia
		// parameters[0]: #4444
		// parameters[1]: +ia

		// is user mode
		if (parameters[0].at(0) != '#')
			return;

		// There is no modestring
		if (parameters.size() < 2)
		{
			// RPL_CHANNELMODEIS (324)
			// RPL_CREATIONTIME (329)
			return;
		}

		std::deque<Channel>::iterator iterChannel = this->channels.begin();
		for (; iterChannel < this->channels.end(); ++iterChannel)
		{
			if (iterChannel->getName() == parameters[0])
			{
				// There is no permission
				if (iterChannel->isOperator(*iterUser) == false)
				{
					// ERR_CHANOPRIVSNEEDED (482)
					return;
				}

				iterChannel->mode(parameters, iterUser);
			}
		}
		// Thers in no Channel
		if (iterChannel == this->channels.end())
		{
			// ERR_NOSUCHCHANNEL (403)
			return;
		}
	}

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