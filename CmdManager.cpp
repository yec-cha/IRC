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
	}

	// std::cout << "cmd : " << command << std::endl;
	// std::cout << "parameters --------------------\n" << "size : " << parameters.size() << std::endl;
	// for (size_t i = 0; i < parameters.size(); i++)
	// {
	// 	std::cout << i << ": [" << parameters[i].c_str() << "]" << std::endl;
	// }
	// std::cout << "----------------------------------\n";

	if (!user->getIsRegistered())
	{
		beforeRegisteredMsg(command, parameters, user);
	}
	else
	{
		afterRegisteredMsg(command, parameters, user);
	}
};

void CmdManager::cmd_JOIN(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters.size() < 1)
	{
		ErrManager::ERR_NEEDMOREPARAMS_461(iterUser->getSocket(), iterUser->getNickName(), "JOIN");
		return;
	}

	std::map<std::string, std::string> channelList;

	std::vector<std::string> channelName;
	std::vector<std::string> key;

	std::stringstream ssName(parameters[0]);
	std::string token;

	while (std::getline(ssName, token, ','))
	{
		if (token.empty() == false)
		{
			if (token.at(0) != '#')
			{
				ErrManager::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), token);
				return;
			}
			channelName.push_back(token);
		}
	}

	if (parameters.size() > 2)
	{
		std::stringstream ssKey(parameters[1]);
		std::string token2;
		while (std::getline(ssKey, token2, ','))
		{
			if (token2.empty() == false) {
				key.push_back(token2);
			}
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
				if (iterChannel->getInviteBool())
				{
					if (!iterChannel->isInvited(*iterUser))
					{
						ErrManager::send_473(iterUser->getSocket(), iterUser->getNickName(), it->first);
						break;
					}
				}
				if (iterChannel->getKeyBool())
				{
					if (it->second != iterChannel->getPassword())
					{
						ErrManager::send_475(iterUser->getSocket(), iterUser->getNickName(), it->first);
						break;
					}
				}
				if (iterChannel->getLimit() > 0)
				{
					if (iterChannel->getLimit() > iterChannel->getSize())
						iterChannel->addUser(*iterUser);
					else
						ErrManager::send_471(iterUser->getSocket(), iterUser->getNickName(), it->first);
				}
				else
					iterChannel->addUser(*iterUser);
				break;
			}
		}
		if (iterChannel == this->channels.end())
		{
			Channel newChannel(it->first, *iterUser);
			this->channels.push_back(newChannel);
		}
	}
}

void CmdManager::cmd_MODE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters.size() < 1)
		return ;
	
	const std::string channel(parameters[0]);
	if (parameters[0][0] != '#')
		return;

	if (parameters.size() < 2)
		return RPL_CHANNELMODEIS_324(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	const std::string modestring(parameters[1]);	
	std::deque<Channel>::iterator iterChannel = this->channels.begin();
	for (; iterChannel < this->channels.end(); ++iterChannel)
	{
		if (iterChannel->getName() == parameters[0])
		{
			if (iterChannel->isOperator(*iterUser) == false)
				return ErrManager::ERR_CHANOPRIVSNEEDED_482(iterUser->getSocket(), iterUser->getNickName(), iterChannel->getName());
			return iterChannel->mode(parameters, iterUser);
		}
	}
	if (iterChannel == this->channels.end())
		return ErrManager::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), channel);
}

void CmdManager::cmd_INVITE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	const std::string user(parameters[0]);
	const std::string channel(parameters[1]);

	if (parameters.size() != 2)
		return ErrManager::ERR_NEEDMOREPARAMS_461(iterUser->getSocket(), iterUser->getNickName(), "INVITE");

	std::deque<Channel>::iterator iterChannel = this->channels.begin();
	for (; iterChannel != this->channels.end(); ++iterChannel) {
		if (iterChannel->getName() == channel)
			break ;
	}
	if (iterChannel == this->channels.end())
		return ErrManager::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	if (iterChannel->isInChannel(*iterUser) == false)
		return ErrManager::ERR_NOTONCHANNEL_442(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	if (iterChannel->getInviteBool() == true)
		if (iterChannel->isOperator(*iterUser) == false)
			return ErrManager::ERR_CHANOPRIVSNEEDED_482(iterUser->getSocket(), iterUser->getNickName(), channel);

	if (iterChannel->isInChannel(user) == true)
		return ErrManager::ERR_USERONCHANNEL_443(iterUser->getSocket(), iterUser->getNickName(), user, channel);

	const std::string RPL_INVITING = "341 " + iterUser->getNickName() + " " + user + " " + channel + "\n";
	sendClient(iterUser->getSocket(), RPL_INVITING);

	const std::string INVITING = ":" + iterUser->getNickName() + " INVITE " + user + " " + channel + "\n";
	for (size_t i = 0; i < users.size(); i++) {
		if (users[i].getNickName() == user)
			sendClient(users[i].getSocket(), INVITING);
	}
}

void CmdManager::cmd_KICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	const std::string channel(parameters[0]);
	const std::string user(parameters[1]);
	
	if (parameters.size() < 2)
		return ErrManager::ERR_NEEDMOREPARAMS_461(iterUser->getSocket(), iterUser->getNickName(), "KICK");
	
	std::deque<Channel>::iterator iterChannel = this->channels.begin();
	for (; iterChannel != this->channels.end(); ++iterChannel) {
		if (iterChannel->getName() == channel)
			break ;
	}
	
	if (iterChannel == this->channels.end())
		return ErrManager::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	if (iterChannel->isOperator(*iterUser) == false)
		return ErrManager::ERR_CHANOPRIVSNEEDED_482(iterUser->getSocket(), iterUser->getNickName(), channel);

	if (iterChannel->isInChannel(user) == false)
		return ErrManager::ERR_USERNOTINCHANNEL_441(iterUser->getSocket(), iterUser->getNickName(), user, channel);
	
	if (iterChannel->isInChannel(*iterUser) == false)
		return ErrManager::ERR_NOTONCHANNEL_442(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	std::string KICK;
	if (parameters.size() == 3) {
		const std::string comment(parameters[2]);
		KICK = "KICK " + channel + " " + user + " :" + comment + "\n";
	}
	else
		KICK = "KICK " + channel + " " + user + "\n";
	for (std::size_t i = 0; i < users.size(); ++i) {
		if (users[i].getNickName() == parameters[1])
			sendClient(users[i].getSocket(), KICK);
	}
}

void CmdManager::cmd_PRIVMSG(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters.size() < 2)
		return ;
	
	std::stringstream ssTarget(parameters[0]);
	std::string target;

	while (std::getline(ssTarget, target, ','))
	{
		if (target.empty() == false)
		{
			if (target.at(0) == '#')
			{
				std::deque<Channel>::iterator iter = this->channels.begin();
				for (; iter != this->channels.end(); ++iter)
				{
					if (iter->getName() == target)
					{
						std::string privmsgCH = ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "PRIVMSG " + parameters[0] + " :" + parameters[1] + "\n";
						iter->sendPrivMsg_(privmsgCH, iterUser->getSocket(), MSG_DONTWAIT);
						break ;
					}
				}
			}
			else
			{
				std::deque<User>::iterator iter = this->users.begin();
				for (; iter < this->users.end(); ++iter)
				{
					if (iter->getNickName() == target) {
						std::string privmsg = ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "PRIVMSG " + parameters[0] + " :" + parameters[1] + "\n";
						sendClient(iter->getSocket(), privmsg);
						break ;
					}
				}
				if (iter == this->users.end())
					ErrManager::ERR_NOSUCHNICK_401(iterUser->getSocket(), iterUser->getNickName(), target);
			}
		}
	}
}

void CmdManager::cmd_NICK(const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	// ERR_NONICKNAMEGIVEN (431) "<client> :No nickname given"
	// ERR_ERRONEUSNICKNAME (432) "<client> <nick> :Erroneus nickname"
	// ERR_NICKNAMEINUSE (433) "<client> <nick> :Nickname is already in use"

	if (!iter->getIsPassed())
		return;
	if (parameters.size() < 1)
	{
		ErrManager::send_431(iter->getSocket());
		return;
	}

	if (parameters[0].find_first_of(" #,*!@?:$.") != std::string::npos || parameters[0].size() > 9)
		ErrManager::send_432(iter->getSocket(), parameters[0]);
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
		ErrManager::ERR_NEEDMOREPARAMS_461(iter->getSocket(), iter->getNickName(), "USER");
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
		ErrManager::ERR_NEEDMOREPARAMS_461(iter->getSocket(), iter->getNickName(), "PASS");
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
	std::deque<Channel>::iterator itChannel;

	if (parameters.size() < 1)
		ErrManager::ERR_NEEDMOREPARAMS_461(iter->getSocket(), iter->getNickName(), "TOPIC");
	else if (parameters.size() == 1)
	{
		for (itChannel = channels.begin(); itChannel != channels.end(); itChannel++)
		{
			if (itChannel->getName() == parameters[0])
			{
				if (itChannel->isInChannel(*iter))
				{
					if (itChannel->getTopic().size() != 0)
						send_332(iter->getSocket(), itChannel->getName(), itChannel->getTopic());
					else
						send_331(iter->getSocket(), itChannel->getName());
				}
				else
					ErrManager::ERR_NOTONCHANNEL_442(iter->getSocket(), iter->getNickName(), itChannel->getName());; // you not in channel;ERR_NOTONCHANNEL
				break;
			}
		}
		if (itChannel == channels.end())
			ErrManager::ERR_NOSUCHCHANNEL_403(iter->getSocket(), iter->getNickName(), itChannel->getName());
	}
	else
	{	
		for (itChannel = channels.begin(); itChannel != channels.end(); itChannel++)
		{
			if (itChannel->getName() == parameters[0])
			{
				if (itChannel->isInChannel(*iter))
				{
					if (itChannel->getIsTopic() == true)
					{
						if (itChannel->isOperator(*iter))
						{
							itChannel->setTopic(parameters[1]);
							itChannel->sendAll_("TOPIC " + parameters[1] + "\n", 0);
						}
						else
							ErrManager::ERR_CHANOPRIVSNEEDED_482(iter->getSocket(), iter->getNickName(), itChannel->getName());
					}
					else
					{
						itChannel->setTopic(parameters[1]);
						itChannel->sendAll_(":" + iter->getNickName() + " TOPIC " + itChannel->getName() + " " + parameters[1] + "\n", 0);
					}
				}
				else
					ErrManager::ERR_NOTONCHANNEL_442(iter->getSocket(), iter->getNickName(), itChannel->getName());
				break;
			}
		}
		if (itChannel == channels.end())
			ErrManager::ERR_NOSUCHCHANNEL_403(iter->getSocket(), iter->getNickName(), parameters[0]);
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
		std::string quitMsg = ":ft_IRC QUIT :";
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