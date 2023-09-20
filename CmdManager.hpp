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
		// printInfo(parameters, iterUser);

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
			{
				if (token[0] != '#')
				{
					ErrManager::send_403(iterUser->getSocket(), token);
					return;
				}
				channelName.push_back(token);
			}
		}

		std::istringstream ssKey(parameters[1]);
		if (parameters.size() >= 2)
		{
			std::string token2;
			while (std::getline(ssKey, token2, ','))
			{
				if (!token2.empty())
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

		// int canJoin = 0;
		// int requiredJoin = 0;
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
					if (iterChannel->getLimit() > 0 && iterChannel->getLimit() > iterChannel->getSize())
						iterChannel->addUser(*iterUser);
					else
					{
						ErrManager::send_471(iterUser->getSocket(), iterUser->getNickName(), it->first);
					}
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

		// There is no modestring
		if (parameters.size() < 2)
		{
			// RPL_CHANNELMODEIS (324)
			// RPL_CREATIONTIME (329)
			return;
		}

		// is user mode
		if (parameters[0][0] != '#')
			return;
		//if (parameters[0][0] != '#')
		//	return;

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

	void cmd_INVITE(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
	{
		// Command: INVITE
		// Parameters: <nickname> <channel>

		// if there is no channel
		// ERR_NOSUCHCHANNEL

		// for (size_t i = 0; i < count; i++)
		//{
		//	/* code */
		// }

		// if invitor is not channel member
		// ERR_NOTONCHANNEL

		// invite-only mode set, and the user is not a channel operator.
		// ERR_CHANOPRIVSNEEDED

		// If the user is already on the target channel
		// ERR_USERONCHANNEL

		// When the invite is successful
		// the server MUST send a RPL_INVITING numeric to the command issuer
		// RPL_INVITING (341)
		// "<client> <nick> <channel>"
		std::string command = "341";
		std::string client = iterUser->getNickName(); // 초대한 사람
		// std::string nick = iterUser->getNickName(); // 초대받은 사람
		std::string nick = parameters[0]; // 초대받은 사람
		std::string channel = parameters[1];
		std::string RPL_INVITING = command + " " + client + " " + nick + " " + channel + "\n";
		// send(iterUser->getSocket(), RPL_INVITING.c_str(), RPL_INVITING.size(), 0);
		sendClient(iterUser->getSocket(), RPL_INVITING);

		// INVITE message, with the issuer as <source>, to the target user
		// Command: INVITE
		// Parameters: <nickname> <channel>
		std::string command_ = "INVITE";
		std::string nickname = parameters[0];
		std::string INVITING = ":" + iterUser->getNickName() + " " + command_ + " " + nickname + " " + channel + "\n";

		for (size_t i = 0; i < users.size(); i++)
		{
			if (users[i].getNickName() == nickname)
			{
				sendClient(users[i].getSocket(), INVITING);
			}
		}
	}

	void cmd_KICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
	{
		std::string command = "353";
		std::string client = iterUser->getNickName(); // 초대한 사람
		// std::string nick = iterUser->getNickName(); // 초대받은 사람
		std::string nick = parameters[1]; // 초대받은 사람
		std::string channel = parameters[0];
		std::string RPL_INVITING = command + " " + client + " " + nick + " " + channel + "\n";
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