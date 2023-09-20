#include "CmdManager.hpp"
#include "Server.hpp"

void CmdManager::sendClient(int socket, const std::string msg)
{
	if (send(socket, msg.c_str(), msg.size(), 0) == -1)
	{
		std::cout << BOLDRED << "Send message failed" << RESET << std::endl;
		return;
	}
	std::cout << "server (to " << socket << ") : " << BLUE << msg << RESET << std::endl;
};

CmdManager::CmdManager(std::deque<User> &_users, std::deque<Channel> &_channels, const std::string &_pass) : users(_users), channels(_channels), pass(_pass){};

CmdManager::~CmdManager(){};

void CmdManager::exeCmd(std::string msg, std::deque<User>::iterator &user)
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

void CmdManager::cmd_NICK(const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	if (!iter->getIsPassed())
		return;
	if (parameters.size() < 1)
	{
		ErrManager::send_431(iter->getSocket());
		return;
	}
	// if ()
	//	ErrManager::send_432(iter->getSocket(), iter->getNickName());
	for (std::deque<User>::iterator it = users.begin(); it != users.end(); it++)
	{
		if (it->getNickName() == *parameters.begin())
		{
			ErrManager::send_433(iter->getSocket(), iter->getNickName(), *parameters.begin());
			return;
		}
	}

	if (iter->getHasNick())
	{
		// const std::string response = ":ft_IRC NICK " + parameters[0] + " :changed name\n";
		//: 이전닉네임!사용자@호스트 NICK :새로운닉네임
		const std::string response = ":" + iter->getNickName() + "!" + iter->getUserName() + "@" + iter->getHostName() + " NICK :" + parameters[0] + "\r\n";
		iter->setNickName(parameters[0]);
		sendClient(iter->getSocket(), response);
	}
	else
	{
		iter->setNickName(parameters[0]);
		std::cout << iter->getSocket() << ": set NICK to [" << parameters[0] << "] \n";
	}
}

void CmdManager::cmd_USER(const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	if (!iter->getIsPassed())
		return;

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

void CmdManager::cmd_PASS(const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	if (parameters.size() < 1)
		ErrManager::send_461(iter->getSocket(), "PASS");
	else if (iter->getIsPassed())
		ErrManager::send_462(iter->getSocket());
	else if (parameters[0] == pass)
	{
		iter->setIsPassed(true);
	}
	else
	{
		ErrManager::send_464(iter->getSocket());
		iter->endCilent();
	}
}

void CmdManager::cmd_TOPIC(const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	if (parameters.size() < 1)
		ErrManager::send_461(iter->getSocket(), "TOPIC");
	else if (parameters.size() == 1)
	{
		std::deque<Channel>::iterator itChannel;
		for (itChannel = channels.begin(); itChannel != channels.end(); itChannel++)
		{
			if (itChannel->getName() == parameters[0])
			{
				if (itChannel->getTopic().size() != 0)
					send_332(iter->getSocket(), itChannel->getName(), itChannel->getTopic());
				else
					send_331(iter->getSocket(), itChannel->getName());
				break;
			}
		}
		if (itChannel == channels.end())
			;
	}
	else
	{
		std::deque<Channel>::iterator itChannel;
		for (itChannel = channels.begin(); itChannel != channels.end(); itChannel++)
		{
			if (itChannel->getName() == parameters[0])
			{
				if (itChannel->isInChannel(*iter))
				{
					//if (topicBool == true)
					//{
					//	if (itChannel->isOperator(*iter))
					//	{
					//		; // change topic
					//	}
					//	else
					//	{
					//		; // you are not op in that CH ERR_CHANOPRIVSNEEDED
					//	}
					//}
					//else
					//{
					//	; // change topic
					//}
				}
				else
					; // you not in channel;ERR_NOTONCHANNEL
				break;
			}
		}
		if (itChannel == channels.end())
			;
	}
}
void CmdManager::beforeRegisteredMsg(std::string &cmd, const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	if (cmd == "PASS")
		cmd_PASS(parameters, iter);
	else if (cmd == "NICK")
		cmd_NICK(parameters, iter);
	else if (cmd == "USER")
		cmd_USER(parameters, iter);
	else if (cmd == "CAP")
	{
		if (parameters.size() > 0)
			if (*parameters.begin() != "END")
				sendClient(iter->getSocket(), "CAP * LS :\n");
	}
	else
		ErrManager::send_451(iter->getSocket());

	if (iter->getIsRegistered())
	{
		sendClient(iter->getSocket(), "001 " + iter->getNickName() + " :Welcome to the Internet Relay Network " + iter->getNickName() + "!" + iter->getUserName() + "@" + iter->getHostName() + "\n");
		sendClient(iter->getSocket(), "002 " + iter->getNickName() + " :Your host is ft_IRC, running version " + VERSION + "\n");
		sendClient(iter->getSocket(), "003 " + iter->getNickName() + " :This server was created 2023.09.07\n");
		sendClient(iter->getSocket(), "004 " + iter->getNickName() + " :ft_IRC " + VERSION + " +i +itkol\n");
		sendClient(iter->getSocket(), "Mode " + iter->getNickName() + " +i\n");
	}
};

void CmdManager::afterRegisteredMsg(std::string &cmd, const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	if (cmd == "PING")
	{
		if (parameters.size() < 1)
			ErrManager::send_409(iter->getSocket());
		else
		{
			sendClient(iter->getSocket(), "PONG " + parameters[0] + "\n");
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
		cmd_JOIN(parameters, iter);
	if (cmd == "PRIVMSG")
		cmd_PRIVMSG(parameters, iter);
	if (cmd == "MODE")
		cmd_MODE(parameters, iter);
	if (cmd == "TOPIC")
		cmd_TOPIC(parameters, iter);
	if (cmd == "INVITE")
		cmd_INVITE(parameters, iter);
	if (cmd == "KICK")
		cmd_KICK(parameters, iter);
};