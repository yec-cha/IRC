#ifndef CMDMANAGER_HPP
#define CMDMANAGER_HPP

#include "User.hpp"
#include "Channel.hpp"
#include "ErrManager.hpp"
#include <vector>
#include <string>

class CmdManager
{
private:
	std::vector<User> &users;
	std::vector<Channel> &channels;

public:
	CmdManager(std::vector<User> &_users, std::vector<Channel> &_channels) : users(_users), channels(_channels){};
	~CmdManager(){};

	void exeCmd(std::string msg, std::vector<User>::iterator &user)
	{
		(void)channels;
		std::string command;
		std::vector<std::string> parameters;

		if (*msg.rbegin() == '\n')
			msg.pop_back();

		while (*msg.begin() == ' ')
			msg.erase(msg.begin());
		while (*(msg.end() - 1) == ' ')
			msg.erase(msg.end() - 1);

		if (msg.find(" ") == std::string::npos)
			command = msg;
		else
		{
			command = msg.substr(0, msg.find(" "));
			msg = msg.substr(msg.find(" ") + 1);
		}

		size_t start = 0;

		for (size_t i = 0; i < msg.size(); i++)
		{
			if (msg[i] == ':')
			{
				start++;
				break;
			}
			if (msg[i] == ' ')
			{
				parameters.push_back(msg.substr(start, i - start));
				while (msg[i] == ' ')
					i++;
				start = i;
				i--;
			}
		}
		if (msg.size() != 0)
			parameters.push_back(msg.substr(start));

		// std::cout << "command : " << command << std::endl;

		if (!user->getIsRegistered())
		{
			beforeRegisteredMsg(command, parameters, user);
		}
		else
		{
			afterRegisteredMsg(command, parameters, user);
		}
	};

	void cmd_NICK(std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
	{
		if (parameters.size() < 1)
		{
			ErrManager::send_431(iter->getSocket());
			return;
		}
		// if ()
		//	ErrManager::send_432(iter->getSocket(), iter->getNickName());
		for (std::vector<User>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			if (iter->getNickName() == *parameters.begin())
			{
				ErrManager::send_433(iter->getSocket(), iter->getNickName());
				return;
			}
		}

		if (iter->getHasNick())
		{
			const std::string response = iter->getNickName() + " NICK " + parameters[0] + "\n";
			iter->setNickName(parameters[0]);
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}
		else
		{
			iter->setNickName(parameters[0]);
		}
	}

	void cmd_USER(std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
	{
		if (parameters.size() < 4)
			ErrManager::send_461(iter->getSocket(), "USER");
		else if (iter->getHasUser())
			ErrManager::send_462(iter->getSocket());
		else
		{
			iter->setUserName(parameters[0]);
			iter->setHostName(parameters[1]);
			iter->setServerName(parameters[2]);
			iter->setRealName(parameters[3]);
		}
	}

	void cmd_PASS(std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
	{
		if (parameters.size() < 1)
			ErrManager::send_461(iter->getSocket(), "PASS");
		else if (iter->getHasUser())
			ErrManager::send_462(iter->getSocket());
		else
		{
			iter->setUserName(parameters[0]);
			iter->setHostName(parameters[1]);
			iter->setServerName(parameters[2]);
			iter->setRealName(parameters[3]);
		}
	}

	void beforeRegisteredMsg(std::string &cmd, std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
	{
		std::string response;

		if (cmd == "NICK")
			cmd_NICK(parameters, iter);
		else if (cmd == "PASS")
			cmd_PASS(parameters, iter);
		else if (cmd == "USER")
			cmd_USER(parameters, iter);
		else
		{
			response = "451 : client must be registered\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}

		if (iter->getIsRegistered())
		{
			response = "001 " + iter->getNickName() + " :Welcome to the Internet Relay Network\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "002 :Your host is ft_irc, running version 1\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "003 :This server was created 2022.3.18\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "004 :ft_irc 1 +i +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "Mode " + iter->getNickName() + " +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}
	};

	void afterRegisteredMsg(std::string &cmd, std::vector<std::string> parameters, std::vector<User>::iterator &iter)
	{
		if (cmd == "NICK")
			cmd_NICK(parameters, iter);
	};
};

#endif