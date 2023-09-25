#include "Channel.hpp"

Channel::Channel() {}

Channel::Channel(const std::string &name, User &user)
: name_(name), invite_(false), key_(false), topic_(false)
{
	this->users_.insert(std::pair<int, std::pair<int, User *> >(user.getSocket(), std::pair<int, User *>(1, &user)));
	welcomeChannel(user);
}

Channel::Channel(const Channel &channel)
: name_(channel.name_), invite_(false), key_(false), topic_(false)
{
	*this = channel;
}

Channel& Channel::operator=(const Channel &channel)
{
	if (this != &channel)
	{
		this->users_ = channel.users_;
		this->invitedUser = channel.invitedUser;
		this->name_ = channel.name_;
		this->topicStr_ = channel.topicStr_;
		this->password_ = channel.password_;
		this->invite_ = channel.invite_;
		this->key_ = channel.key_;
		this->topic_ = channel.topic_;
		this->userLimit_ = channel.userLimit_;
	}
	return *this;
}

Channel::~Channel() {}

void Channel::addInvitedUser(const std::string &name)
{
	invitedUser.push_back(name);
}

bool Channel::isInvited(User &user)
{
	for (size_t i = 0; i < invitedUser.size(); i++)
	{
		if (invitedUser[i] == user.getNickName())
			return true;
	}
	return false;
}

int Channel::getLimit() const
{
	return userLimit_;
}

bool Channel::getInviteBool() const
{
	return invite_;
}

bool Channel::getKeyBool() const
{
	return key_;
}

bool Channel::getIsTopic() const
{
	return topic_;
}
 
const std::string& Channel::getPassword() const
{
	return password_;
}

void Channel::send_(int socket, const std::string &buffer, int flags)
{
	std::cout << BLUE "server to (" << socket << ") : " << buffer << RESET << std::endl;
	send(socket, buffer.c_str(), buffer.size(), flags);
}

void Channel::sendAll_(const std::string &buffer, int flags)
{
	for (std::map<int, std::pair<int, User *> >::const_iterator iter = users_.begin(); iter != this->users_.end(); ++iter)
		send_(iter->first, buffer, flags);
}

void Channel::sendPrivMsg_(const std::string &buffer, int socket, int flags)
{
	for (std::map<int, std::pair<int, User *> >::iterator iter = this->users_.begin(); iter != this->users_.end(); ++iter)
	{
		if (socket != iter->first)
			this->send_(iter->first, buffer, flags);
	}
}

void Channel::sendChannelPRIVMSG(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	for (std::map<int, std::pair<int, User *> >::iterator iter = this->users_.begin(); iter != this->users_.end(); ++iter)
	{
		if (iterUser->getSocket() != iter->first)
		{
			std::string privmsg = ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "PRIVMSG " + parameters[0] + " :" + parameters[1] + "\n";
			this->send_(iter->first, privmsg, MSG_DONTWAIT);
		}
	}
}

void Channel::welcomeChannel(User &user)
{
	std::string JOIN = ":" + user.getNickName() + "!" + user.getNickName() + "@" + user.getHostName() + " " + "JOIN" + " " + this->name_ + "\n";
	this->sendAll_(JOIN, MSG_DONTWAIT);

	std::string TOPIC_command = "332";
	std::string TOPIC = ":ft_IRC " + TOPIC_command + " " + user.getNickName() + " " + this->name_ + " :" + this->topicStr_ + "\n";
	this->send_(user.getSocket(), TOPIC, MSG_DONTWAIT);

	std::string NAMREPLY_command = "353";
	std::string NAMREPLY_symbol = "=";
	std::string NAMREPLY = ":ft_IRC " + NAMREPLY_command + " " + user.getNickName() + " " + NAMREPLY_symbol + " " + this->name_ + " :";

	std::string nameOfClient;
	for (std::map<int, std::pair<int, User *> >::iterator it = users_.begin(); it != users_.end(); it++)
	{
		if (it->second.first)
			nameOfClient = "@" + it->second.second->getNickName() + " ";
		else
			nameOfClient = it->second.second->getNickName() + " ";
		NAMREPLY += nameOfClient;
	}

	NAMREPLY += "\n";
	this->send_(user.getSocket(), NAMREPLY, MSG_DONTWAIT);

	std::string ENDOFNAMES_command = "366";
	std::string ENDOFNAMES = ":ft_IRC " + ENDOFNAMES_command + " " + user.getNickName() + " " + this->name_ + " :End of /NAMES list\n";
	this->send_(user.getSocket(), ENDOFNAMES, MSG_DONTWAIT);
}

const std::string Channel::MODE(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters.size() > 2)
		return ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "MODE " + parameters[1] + " " + parameters[2] + "\n";
	else
		return ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "MODE " + parameters[1] + "\n";
}

const std::string Channel::NOTICE(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters.size() > 2)
		return ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "MODE " + parameters[0] + " :" + parameters[1] + " " + parameters[2] + "\n";
	else
		return ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + iterUser->getHostName() + " " + "MODE " + parameters[0] + " :" + parameters[1] + "\n";
}

void Channel::inviteMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters[1].at(0) == '+')
	{
		if (this->invite_ == false)
		{
			this->invite_ = true;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
	else if (parameters[1].at(0) == '-')
	{
		if (this->invite_ == true)
		{
			this->invite_ = false;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
}

void Channel::topicMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters[1].at(0) == '+')
	{
		if (this->topic_ == false)
		{
			this->topic_ = true;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
	else if (parameters[1].at(0) == '-')
	{
		if (this->topic_ == true)
		{
			this->topic_ = false;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
}

void Channel::keyMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters[1].at(0) == '+')
	{
		if (this->key_ == false && parameters[2].size() != 0)
		{
			this->key_ = true;
			this->password_ = parameters[2];
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
	else if (parameters[1].at(0) == '-')
	{
		if (this->key_ == true)
		{
			this->key_ = false;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
}

void Channel::operatorMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	std::map<int, std::pair<int, User *> >::iterator iter = this->users_.begin();
	for (; iter != this->users_.end(); ++iter) {
		if (iter->second.second->getNickName() == parameters[2]) {
			if (parameters[1].at(0) == '+')
				iter->second.first = 1;
			else if (parameters[1].at(0) == '-')
				iter->second.first = 0;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
			return ;
		}
	}
}

void Channel::userLimitMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	if (parameters[1].at(0) == '+')
	{
		int value = std::atoi(parameters[2].c_str());
		if (value > 0)
		{
			this->userLimit_ = value;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
	else if (parameters[1].at(0) == '-')
	{
		if (this->userLimit_ != 0)
		{
			this->userLimit_ = 0;
			this->send_(iterUser->getSocket(), this->MODE(parameters, iterUser), MSG_DONTWAIT);
			this->sendAll_(this->NOTICE(parameters, iterUser), MSG_DONTWAIT);
		}
	}
}

bool Channel::isInChannel(User &user) const
{
	if (users_.find(user.getSocket()) == users_.end())
		return false;
	return true;
}

bool Channel::isInChannel(const std::string& user) const
{
	std::map<int, std::pair<int, User*> >::const_iterator iter = this->users_.begin();
	for (; iter != this->users_.end(); ++iter) {
		if (iter->second.second->getNickName() == user)
			return true;
	}
	return false;
}

bool Channel::isOperator(User &user) const
{
	std::map<int, std::pair<int, User *> >::const_iterator iter = users_.find(user.getSocket());
	if (iter == users_.end())
		return false;
	if (iter->second.first == 1)
		return true;
	return false;
}

void Channel::mode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser)
{
	std::cout << "MODE PARSING START" << std::endl;
	std::string modeOp("");
	std::string para("");

	for (size_t i = 1; i < parameters.size(); i++)
	{
		if (parameters[i].size() > 0)
		{
			if (parameters[i].at(0) == '+' || parameters[i].at(0) == '-')
			{
				if (parameters[i].size() > 1)
					modeOp += parameters[i];
			}
			else
				para += parameters[i] + " ";
		}
	}

	std::istringstream ss(para);
	std::string token;
	int isPlus = 0;

	std::vector<std::string> paras;

	for (size_t i = 0; i < modeOp.size(); i++)
	{
		if (modeOp[i] == '+')
			isPlus = 1;
		else if (modeOp[i] == '-')
			isPlus = 0;
		else
		{
			paras.clear();
			paras.push_back(parameters[0]);
			switch (modeOp[i])
			{
			case 'i':
				if (isPlus)
					paras.push_back("+i");
				else
					paras.push_back("-i");
				this->inviteMode(paras, iterUser);
				break;
			case 't':
				if (isPlus)
					paras.push_back("+t");
				else
					paras.push_back("-t");
				this->topicMode(paras, iterUser);
				break;
			case 'k':
				if (isPlus)
				{
					paras.push_back("+k");
					std::getline(ss, token, ' ');
					if (token.size() != 0)
					{
						paras.push_back(token);
						this->keyMode(paras, iterUser);
					}
				}
				else
				{
					paras.push_back("-k");
					this->keyMode(paras, iterUser);
				}
				break;
			case 'o':
				if (isPlus)
					paras.push_back("+o");
				else
					paras.push_back("-o");
				std::getline(ss, token, ' ');
				if (token.size() != 0)
				{
					paras.push_back(token);
					this->operatorMode(paras, iterUser);
				}
				break;
			case 'l':
				if (isPlus)
				{
					paras.push_back("+l");
					std::getline(ss, token, ' ');
					if (token.size() != 0)
					{
						paras.push_back(token);
						this->userLimitMode(paras, iterUser);
					}
				}
				else
				{
					paras.push_back("-l");
					this->userLimitMode(paras, iterUser);
				}
				break;
			default:
				break;
			}
		}
	}
}

void Channel::addUser(User &user)
{
	this->users_.insert(std::pair<int, std::pair<int, User *> >(user.getSocket(), std::pair<int, User *>(0, &user)));
	welcomeChannel(user);
}

void Channel::delUser(User &user)
{
	this->users_.erase(user.getSocket());
}

void Channel::setTopic(const std::string &topic)
{
	this->topicStr_ = topic;
}

void Channel::setPass(const std::string &pass)
{
	this->topicStr_ = pass;
}

const std::string &Channel::getName() const
{
	return this->name_;
}

const std::string &Channel::getTopic() const
{
	return this->topicStr_;
}

int Channel::getSize() const
{
	return this->users_.size();
}

void Channel::deleteUser(const int socket)
{
	std::map<int, std::pair<int, User *> >::iterator iter = users_.find(socket);
	if (iter != users_.end())
	{
		sendAll_(":" + iter->second.second->getNickName() + "!" + iter->second.second->getUserName() + "@" + iter->second.second->getHostName() + " PART " + name_ + "\n", MSG_DONTWAIT);
		users_.erase(users_.find(socket));
	}
}
