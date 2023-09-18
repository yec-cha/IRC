#include "CmdManager.hpp"

void CmdManager::sendClient(int socket, const std::string msg)
{
	if (send(socket, msg.c_str(), msg.size(), 0) == -1)
	{
		std::cout << BOLDRED << "Send message failed" << RESET << std::endl;
		return;
	}
	std::cout << "server (to " << socket << ") : " << BLUE << msg << RESET << std::endl;
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
	while (*(msg.end() - 1) == ' ' || *(msg.end() - 1) == '\r' || *(msg.end() - 1) == '\n')
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
	{
		parameters.push_back(msg.substr(start));
	}

	// std::cout << "cmd : " << command << std::endl;
	// std::cout << "parameters --------------------\n";
	// for (size_t i = 0; i < parameters.size(); i++)
	// {
	// 	std::cout << parameters[i].c_str() << "E" << std::endl;
	// }
	// std::cout << "----------------------------------\n";

	// std::cout << user->getIsRegistered() << std::endl;

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
		const std::string response = ":" + iter->getNickName() + " NICK " + parameters[0] + "\n";
		// const std::string response = "NICK " + parameters[0] + "\n";
		iter->setNickName(parameters[0]);
		sendClient(iter->getSocket(), response);
	}
	else
	{
		iter->setNickName(parameters[0]);

		std::cout << iter->getSocket() << ": set NICK to [" << parameters[0] << "] \n";
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

		std::cout << iter->getSocket() << " : set USER\n";
	}
}

void CmdManager::cmd_PASS(const std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
{
	iter->setIsPassed(true);
	return;

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
		sendClient(iter->getSocket(), "001 " + iter->getNickName() + " :Welcome to the Internet Relay Network\n");
		sendClient(iter->getSocket(), "002 " + iter->getNickName() + " :Your host is ft_irc, running version 1\n");
		sendClient(iter->getSocket(), "003 " + iter->getNickName() + " :This server was created 2022.3.18\n");
		sendClient(iter->getSocket(), "004 " + iter->getNickName() + " :ft_irc 1 +i +i\n");
		sendClient(iter->getSocket(), "Mode " + iter->getNickName() + " +i\n");
	}
};

void CmdManager::afterRegisteredMsg(std::string &cmd, const std::vector<std::string> &parameters, std::vector<User>::iterator &iter)
{
	if (cmd == "PING")
	{
		if (parameters.size() < 1)
			ErrManager::send_409(iter->getSocket());
		else
		{
			sendClient(iter->getSocket(), "PONG " + parameters[0]);
		}
	}
	if (cmd == "NICK")
	{
		cmd_NICK(parameters, iter);
	}
	if (cmd == "QUIT")
	{
		std::string quitMsg = "QUIT :";
		iter->endCilent();

		if (parameters.size() > 0)
			quitMsg = quitMsg + parameters[0];
		else
			quitMsg = quitMsg + "good bye :)\n";
		sendClient(iter->getSocket(), quitMsg);
	}
	if (cmd == "JOIN")
	{
		// std::cout << "[TEST] Join ----------------" << std::endl;
		// std::cout << "cmd: " << cmd << std::endl;
		// for (size_t i = 0; i < parameters.size(); ++i)
		// 	std::cout << "params: " << parameters[i] << std::endl;
		// std::cout << "----------------------" << std::endl;

		cmd_JOIN(parameters, iter);

		// std::cout << "[TEST] Join ----------------" << std::endl;
	}
	if (cmd == "PRIVMSG")
		cmd_PRIVMSG(parameters, iter);
};