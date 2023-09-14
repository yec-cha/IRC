#ifndef CMDMANAGER_HPP
#define CMDMANAGER_HPP

#include "User.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include <vector>
#include <string>

class cmdManager
{
private:
	std::vector<User> &users;
	std::vector<Channel> &channels;

public:
	cmdManager(std::vector<User> &_users, std::vector<Channel> &_channels) : users(_users), channels(_channels){};
	~cmdManager(){};

	void exeCmd(std::string msg, std::vector<User>::iterator &user)
	{
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

		int start = 0;

		for (int i = 0; i < msg.size(); i++)
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

		if (!user->isRegistered)
		{
			beforeRegisteredMsg(command, parameters, user);
		}
		else
		{
			afterRegisteredMsg(command, parameters, user);
		}
	};

	void beforeRegisteredMsg(std::string &cmd, std::vector<std::string> parameters, std::vector<User>::iterator &iter)
	{
		std::string response;

		if (cmd == "NICK")
		{
		}
		else if (cmd == "PASS")
		{
			iter->isPassed = true;
		}
		else if (cmd == "USER")
		{
			iter->isRegistered = true;
		}
		else
		{
			response = "451 : client must be registered\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}

		if (iter->isRegistered)
		{
			response = "001 " + iter->getnickname() + " :Welcome to the Internet Relay Network\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "002 :Your host is ft_irc, running version 1\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "003 :This server was created 2022.3.18\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "004 :ft_irc 1 +i +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "Mode " + iter->getnickname() + " +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}
	};

	void afterRegisteredMsg(std::string &cmd, std::vector<std::string> parameters, std::vector<User>::iterator &iter)
	{
		std::string response;

		if (cmd == "NICK")
		{
		}
		else if (cmd == "PASS")
		{
			iter->isPassed = true;
		}
		else if (cmd == "USER")
		{
			iter->isRegistered = true;
		}
		else
		{
			response = "451 : client must be registered\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}

		if (iter->isRegistered)
		{
			response = "001 " + iter->getnickname() + " :Welcome to the Internet Relay Network\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "002 :Your host is ft_irc, running version 1\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "003 :This server was created 2022.3.18\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "004 :ft_irc 1 +i +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "Mode " + iter->getnickname() + " +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}
	};
};

#endif