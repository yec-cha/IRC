#include "CmdManager.hpp"
#include "Server.hpp"

CmdManager::CmdManager(std::deque<User> &_users, std::deque<Channel> &_channels, const std::string &_pass)
: users(_users), channels(_channels), pass(_pass) {}

CmdManager::~CmdManager() {}

void CmdManager::sendClient(int socket, const std::string msg)
{
	if (send(socket, msg.c_str(), msg.size(), 0) == -1)
	{
		std::cout << BOLDRED << "Send message failed" << RESET << std::endl;
		return;
	}
	std::cout << "server (to " << socket << ") : " << BLUE << msg << RESET << std::endl;
};

void CmdManager::exeCmd(std::string msg, std::deque<User>::iterator &user)
{
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
		size_t start = 0;
		
		command = msg.substr(0, msg.find(" "));
		msg = msg.substr(msg.find(" ") + 1);
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
	}
	if (!user->getIsRegistered())
		beforeRegisteredMsg(command, parameters, user);
	else
		afterRegisteredMsg(command, parameters, user);
};

void CmdManager::cmd_JOIN(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters.size() < 1)
		return Message::ERR_NEEDMOREPARAMS_461(iterUser->getSocket(), iterUser->getNickName(), "JOIN");
	
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
				return Message::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), token);
			channelName.push_back(token);
		}
	}
	if (parameters.size() > 2)
	{
		std::stringstream ssKey(parameters[1]);
		std::string token2;
		while (std::getline(ssKey, token2, ','))
		{
			if (token2.empty() == false)
				key.push_back(token2);
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
						Message::ERR_INVITEONLYCHAN_473(iterUser->getSocket(), iterUser->getNickName(), it->first);
						break;
					}
				}
				if (iterChannel->getKeyBool())
				{
					if (it->second != iterChannel->getPassword())
					{
						Message::ERR_BADCHANNELKEY_475(iterUser->getSocket(), iterUser->getNickName(), it->first);
						break;
					}
				}
				if (iterChannel->getLimit() > 0)
				{
					if (iterChannel->getLimit() > iterChannel->getSize())
						iterChannel->addUser(*iterUser);
					else
						Message::ERR_CHANNELISFULL_471(iterUser->getSocket(), iterUser->getNickName(), it->first);
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
		return Message::RPL_CHANNELMODEIS_324(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	const std::string modestring(parameters[1]);	
	std::deque<Channel>::iterator iterChannel = this->channels.begin();
	for (; iterChannel < this->channels.end(); ++iterChannel)
	{
		if (iterChannel->getName() == parameters[0])
		{
			if (iterChannel->isOperator(*iterUser) == false)
				return Message::ERR_CHANOPRIVSNEEDED_482(iterUser->getSocket(), iterUser->getNickName(), iterChannel->getName());
			return iterChannel->mode(parameters, iterUser);
		}
	}
	if (iterChannel == this->channels.end())
		return Message::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), channel);
}

void CmdManager::cmd_INVITE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	const std::string user(parameters[0]);
	const std::string channel(parameters[1]);

	if (parameters.size() != 2)
		return Message::ERR_NEEDMOREPARAMS_461(iterUser->getSocket(), iterUser->getNickName(), "INVITE");

	std::deque<Channel>::iterator iterChannel = this->channels.begin();
	for (; iterChannel != this->channels.end(); ++iterChannel) {
		if (iterChannel->getName() == channel)
			break ;
	}
	if (iterChannel == this->channels.end())
		return Message::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	if (iterChannel->isInChannel(*iterUser) == false)
		return Message::ERR_NOTONCHANNEL_442(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	if (iterChannel->getInviteBool() == true)
		if (iterChannel->isOperator(*iterUser) == false)
			return Message::ERR_CHANOPRIVSNEEDED_482(iterUser->getSocket(), iterUser->getNickName(), channel);

	if (iterChannel->isInChannel(user) == true)
		return Message::ERR_USERONCHANNEL_443(iterUser->getSocket(), iterUser->getNickName(), user, channel);

	const std::string RPL_INVITING = "341 " + iterUser->getNickName() + " " + user + " " + channel + "\n";
	sendClient(iterUser->getSocket(), RPL_INVITING);

	const std::string INVITING = ":" + iterUser->getNickName() + " INVITE " + user + " " + channel + "\n";
	for (size_t i = 0; i < users.size(); i++) {
		if (users[i].getNickName() == user)
		{
			iterChannel->addInvitedUser(users[i].getNickName());
			sendClient(users[i].getSocket(), INVITING);
		}
	}
}

void CmdManager::cmd_KICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
{
	const std::string channel(parameters[0]);
	const std::string user(parameters[1]);
	
	if (parameters.size() < 2)
		return Message::ERR_NEEDMOREPARAMS_461(iterUser->getSocket(), iterUser->getNickName(), "KICK");
	
	std::deque<Channel>::iterator iterChannel = this->channels.begin();
	for (; iterChannel != this->channels.end(); ++iterChannel) {
		if (iterChannel->getName() == channel)
			break ;
	}
	
	if (iterChannel == this->channels.end())
		return Message::ERR_NOSUCHCHANNEL_403(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	if (iterChannel->isOperator(*iterUser) == false)
		return Message::ERR_CHANOPRIVSNEEDED_482(iterUser->getSocket(), iterUser->getNickName(), channel);

	if (iterChannel->isInChannel(user) == false)
		return Message::ERR_USERNOTINCHANNEL_441(iterUser->getSocket(), iterUser->getNickName(), user, channel);
	
	if (iterChannel->isInChannel(*iterUser) == false)
		return Message::ERR_NOTONCHANNEL_442(iterUser->getSocket(), iterUser->getNickName(), channel);
	
	std::string KICK;
	if (parameters.size() == 3) {
		const std::string comment(parameters[2]);
		KICK = "KICK " + channel + " " + user + " :" + comment + "\n";
	}
	else
		KICK = "KICK " + channel + " " + user + "\n";
	for (std::size_t i = 0; i < users.size(); ++i) {
		if (users[i].getNickName() == parameters[1])
		{
			sendClient(users[i].getSocket(), KICK);
			iterChannel->deleteUser(users[i].getSocket());
		}
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
						std::string privmsgCH = ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "PRIVMSG " + target + " :" + parameters[1] + "\n";
						iter->sendPrivMsg_(privmsgCH, iterUser->getSocket(), MSG_DONTWAIT);
						if (parameters[1].at(0) == '!')
							bot(iter, parameters[1]);
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
						std::string privmsg = ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "PRIVMSG " + target + " :" + parameters[1] + "\n";
						sendClient(iter->getSocket(), privmsg);
						break ;
					}
				}
				if (iter == this->users.end())
					Message::ERR_NOSUCHNICK_401(iterUser->getSocket(), iterUser->getNickName(), target);
			}
		}
	}
}

void CmdManager::cmd_NICK(const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	if (!iter->getIsPassed())
		return;
	if (parameters.size() < 1)
		return Message::ERR_NONICKNAMEGIVEN_431(iter->getSocket(), iter->getNickName());

	if (parameters[0].find_first_of(" #,*!@?:$.") != std::string::npos || parameters[0].size() > 9)
		Message::ERR_ERRONEUSNICKNAME_432(iter->getSocket(), iter->getNickName(), parameters[0]);
	for (std::deque<User>::iterator it = users.begin(); it != users.end(); it++)
	{
		if (it->getNickName() == *parameters.begin())
			return Message::ERR_NICKNAMEINUSE_433(iter->getSocket(), iter->getNickName(), *parameters.begin());
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
		Message::ERR_NEEDMOREPARAMS_461(iter->getSocket(), iter->getNickName(), "USER");
	else if (iter->getHasUser())
		Message::ERR_ALREADYREGISTERED_462(iter->getSocket(), iter->getNickName());
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
		Message::ERR_NEEDMOREPARAMS_461(iter->getSocket(), iter->getNickName(), "PASS");
	else if (iter->getIsPassed())
		Message::ERR_ALREADYREGISTERED_462(iter->getSocket(), iter->getNickName());
	else if (parameters[0] == pass)
	{
		iter->setIsPassed(true);
	}
	else
	{
		Message::ERR_PASSWDMISMATCH_464(iter->getSocket(), iter->getNickName());
		iter->endCilent();
	}
}

void CmdManager::cmd_TOPIC(const std::vector<std::string> &parameters, std::deque<User>::iterator &iter)
{
	std::deque<Channel>::iterator itChannel;

	if (parameters.size() < 1)
		Message::ERR_NEEDMOREPARAMS_461(iter->getSocket(), iter->getNickName(), "TOPIC");
	else if (parameters.size() == 1)
	{
		for (itChannel = channels.begin(); itChannel != channels.end(); itChannel++)
		{
			if (itChannel->getName() == parameters[0])
			{
				if (itChannel->isInChannel(*iter))
				{
					if (itChannel->getTopic().size() != 0)
						Message::RPL_TOPIC_332(iter->getSocket(), iter->getNickName(), itChannel->getName(), itChannel->getTopic());
					else
						Message::RPL_NOTOPIC_331(iter->getSocket(), iter->getNickName(), itChannel->getName());
				}
				else
					Message::ERR_NOTONCHANNEL_442(iter->getSocket(), iter->getNickName(), itChannel->getName());; // you not in channel;ERR_NOTONCHANNEL
				break;
			}
		}
		if (itChannel == channels.end())
			Message::ERR_NOSUCHCHANNEL_403(iter->getSocket(), iter->getNickName(), itChannel->getName());
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
							Message::ERR_CHANOPRIVSNEEDED_482(iter->getSocket(), iter->getNickName(), itChannel->getName());
					}
					else
					{
						itChannel->setTopic(parameters[1]);
						itChannel->sendAll_(":" + iter->getNickName() + " TOPIC " + itChannel->getName() + " " + parameters[1] + "\n", 0);
					}
				}
				else
					Message::ERR_NOTONCHANNEL_442(iter->getSocket(), iter->getNickName(), itChannel->getName());
				break;
			}
		}
		if (itChannel == channels.end())
			Message::ERR_NOSUCHCHANNEL_403(iter->getSocket(), iter->getNickName(), parameters[0]);
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
		Message::ERR_NOTREGISTERED_451(iter->getSocket(), iter->getNickName());

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
			Message::ERR_NOORIGIN_409(iter->getSocket(), iter->getNickName());
		else
		{
			sendClient(iter->getSocket(), "PONG " + parameters[0] + "\n");
		}
	}
	else if (cmd == "QUIT")
	{
		std::string quitMsg = ":ft_IRC QUIT :";
		iter->endCilent();

		if (parameters.size() > 0)
			quitMsg = quitMsg + parameters[0];
		else
			quitMsg = quitMsg + "good bye :)\n";
		sendClient(iter->getSocket(), quitMsg);
	}
	else if (cmd == "NICK")
		cmd_NICK(parameters, iter);
	else if (cmd == "JOIN")
		cmd_JOIN(parameters, iter);
	else if (cmd == "PRIVMSG")
		cmd_PRIVMSG(parameters, iter);
	else if (cmd == "MODE")
		cmd_MODE(parameters, iter);
	else if (cmd == "TOPIC")
		cmd_TOPIC(parameters, iter);
	else if (cmd == "INVITE")
		cmd_INVITE(parameters, iter);
	else if (cmd == "KICK")
		cmd_KICK(parameters, iter);
	else
		return Message::ERR_UNKNOWNCOMMAND_421(iter->getSocket(), iter->getNickName(), cmd);
};

void CmdManager::bot(const std::deque<Channel>::iterator &iter, const std::string &msg)
{
	std::string botMsg;
	if (msg == "!hi")
	{
		botMsg = ":bot NOTICE " + iter->getName() + " :hello~~\n";
		iter->sendPrivMsg_(botMsg, -1, MSG_DONTWAIT);
	}
	if (msg == "!help")
	{
		botMsg = ":bot NOTICE " + iter->getName() + " :CMD LIST : KICK INVITE TOPIC MODE(+) NICK JOIN MSG(PRIVMSG)\n";
		iter->sendPrivMsg_(botMsg, -1, MSG_DONTWAIT);
	}
	if (msg == "!help MODE")
	{
		botMsg = ":bot NOTICE " + iter->getName() + " :MODE <channel> {[+|-]|i|t|k|o|l} [parameters]\n";
		iter->sendPrivMsg_(botMsg, -1, MSG_DONTWAIT);
	}
	if (msg == "!HRU")
	{
		botMsg = ":bot NOTICE " + iter->getName() + " :hello~~\n";
		iter->sendPrivMsg_(botMsg, -1, MSG_DONTWAIT);
	}
	if (msg == "!time")
	{
		time_t timer;
		struct tm* t;
		timer = time(0); // 1970년 1월 1일 0시 0분 0초부터 시작하여 현재까지의 초
		t = localtime(&timer); 
		std::stringstream ss;

		ss << (t->tm_year + 1900);
		ss << ".";
		if (t->tm_mon + 1 < 10)
			ss << "0";
		ss << (t->tm_mon + 1);
		ss << ".";
		if (t->tm_mday < 10)
			ss << "0";
		ss << t->tm_mday;
		ss << "_";
		if (t->tm_hour < 10)
			ss << "0";
		ss << t->tm_hour;
		ss << ":";
		if (t->tm_min < 10)
			ss << "0";
		ss << t->tm_min;
		ss << ":";
		if (t->tm_sec < 10)
			ss << "0";
		ss << t->tm_sec;

		std::string result;
		ss >> result;
		
		botMsg = ":bot NOTICE " + iter->getName() + " :" + result + "\n";
		iter->sendPrivMsg_(botMsg, -1, MSG_DONTWAIT);
	}
}
