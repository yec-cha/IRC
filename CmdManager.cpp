#include "CmdManager.hpp"

void CmdManager::sendClient(int socket, const std::string msg)
{
	if (send(socket, msg.c_str(), msg.size(), 0) == -1)
	{
		std::cout << RED << "Send message failed" << RESET << std::endl;
		return ;
	}
	std::cout << "server (to " << socket << ") :" << msg << std::endl;
};

CmdManager::CmdManager(std::vector<User> &_users, std::vector<Channel> &_channels) : users(_users), channels(_channels){};

CmdManager::~CmdManager(){};

void CmdManager::exeCmd(std::string msg, std::vector<User>::iterator &user)
{
	(void)channels;
	std::string command;
	std::vector<std::string> parameters;

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


	std::cout << "cmd : " << command << std::endl;
	std::cout << "parameters --------------------\n";
	for (size_t i = 0; i < parameters.size(); i++)
	{
		std::cout << parameters[i] << "E" << std::endl;
	}
	std::cout << "----------------------------------\n";
	
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

void CmdManager::cmd_NICK(const std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
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

		std::cout << iter->getSocket() << ": set NICK\n";
	}
}

void CmdManager::cmd_USER(const std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
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

		std::cout << iter->getSocket() << ": set USER\n";
	}
}

void CmdManager::cmd_PASS(const std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
{
	iter->setIsPassed(true);
	return ;

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

void CmdManager::beforeRegisteredMsg(std::string &cmd, const std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
{
	if (cmd == "NICK")
		cmd_NICK(parameters, iter);
	else if (cmd == "PASS")
		cmd_PASS(parameters, iter);
	else if (cmd == "USER")
		cmd_USER(parameters, iter);
	else if (cmd == "CAP")
		;
	else
		ErrManager::send_451(iter->getSocket());
	
	if (iter->getIsRegistered())
	{
		std::cout << RED << "REGISTERD" << RESET << std::endl;

		std::cout << "nick :" << iter->getNickName() << std::endl;

		std::string s1 = "001 ";
		std::string s2 = " :Welcome to the Internet Relay Network\n";
		std::string nick = iter->getNickName();

		std::string response = s1 + nick + s2;
		std::cout << response << nick << std::endl;
		sendClient(iter->getSocket(), response);
		sendClient(iter->getSocket(), ("002 :Your host is ft_irc, running version 1\n"));
		sendClient(iter->getSocket(), ("003 :This server was created 2022.3.18\n"));
		sendClient(iter->getSocket(), ("004 :ft_irc 1 +i +i\n"));
		sendClient(iter->getSocket(), ("Mode " + iter->getNickName() + " +i\n"));
	}
};

void CmdManager::afterRegisteredMsg(std::string &cmd, const std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
{
	if (cmd == "NICK")
	{
		std::cout << iter->getSocket() << ":" << *parameters.begin() << std::endl;
	}
};