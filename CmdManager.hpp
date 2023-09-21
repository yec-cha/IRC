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

		std::stringstream ssName(parameters[0]);
		std::string token;

		while (std::getline(ssName, token, ','))
		{
			if (token.empty() == false)
			{
				if (token.at(0) != '#')
				{
					ErrManager::send_403(iterUser->getSocket(), token);
					return;
				}
				channelName.push_back(token);
			}
		}

		std::cout << "SEG1" << std::endl;

		std::stringstream ssKey(parameters[1]);
		if (parameters.size() > 2)
		{
			std::string token2;
			while (std::getline(ssKey, token2, ','))
			{
				if (token2.empty() == false) {
					key.push_back(token2);
				}
			}
		}

		std::cout << "SEG2" << std::endl;

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

	void cmd_PRIVMSG(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
	{
		std::deque<Channel>::iterator iterChannel = this->channels.begin();
		for (; iterChannel < this->channels.end(); ++iterChannel)
		{
			if (iterChannel->getName() == parameters[0])
				iterChannel->sendPRIVMSG(parameters, iterUser);
		}
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
		// if there is not enough params
		// ERR_NEEDMOREPARAMS (461)
		if (parameters.size() != 2)
			return ErrManager::send_461(iterUser->getSocket(), "INVITE");

		// if there is no channel
		// ERR_NOSUCHCHANNEL (403)
		// std::deque
		// this->channels.begin
		// if (parameters.size() != 2)
		// 	return ErrManager::send_461(iterUser->getSocket(), "INVITE");

		// Returned when a client tries to perform a channel-affecting command on a channel which the client isn’t a part of.
		// ERR_NOTONCHANNEL (442) 

		// invite-only mode set, and the user is not a channel operator.
		// ERR_CHANOPRIVSNEEDED (482)

		// If the user is already on the target channel
		// ERR_USERONCHANNEL (443)



		// When the invite is successful
		// RPL_INVITING (341)
		// "<client> <nick> <channel>"
		// the server MUST send a RPL_INVITING numeric to the command issuer
		// std::string command = "341";
		// std::string client = iterUser->getNickName(); // 초대한 사람
		// // std::string nick = iterUser->getNickName(); // 초대받은 사람
		// std::string nick = parameters[0]; // 초대받은 사람
		// std::string channel = parameters[1]; // channel name
		std::string RPL_INVITING = "341 " + iterUser->getNickName() + " " + parameters[0] + " " + parameters[1] + "\n";
		sendClient(iterUser->getSocket(), RPL_INVITING);

		// Command: INVITE
		// Parameters: <nickname> <channel>
		// INVITE message, with the issuer as <source>, to the target user
		// std::string command_ = "INVITE";
		// std::string nickname = parameters[0];
		std::string INVITING = ":" + iterUser->getNickName() + " INVITE " + parameters[0] + " " + parameters[1] + "\n";

		for (size_t i = 0; i < users.size(); i++)
		{
			if (users[i].getNickName() == parameters[0])
			{
				sendClient(users[i].getSocket(), INVITING);
			}
		}
	}

	void cmd_KICK(std::vector<std::string> const &parameters, std::deque<User>::iterator &iterUser)
	{
		// if there is not enough params
		// ERR_NEEDMOREPARAMS (461)

		// if there is no channel
		// ERR_NOSUCHCHANNEL (403)

		// the user is not a channel operator.
		// ERR_CHANOPRIVSNEEDED (482)

		// when the nick isn’t joined to the channel
		// ERR_USERNOTINCHANNEL (441) 
		
		// a client tries to perform a channel-affecting command on a channel which the client isn’t a part of
		// ERR_NOTONCHANNEL (442) 

		// Bad channel mask
		// ERR_BADCHANMASK (476)
  

		// KICK message
		// Command: KICK
		// Parameters: <channel> <user> *( "," <user> ) [<comment>]
		std::string command = "KICK";
		std::string channel = parameters[0];
		std::string user = parameters[1]; // be kicked
		// std::string user2 = parameters[2]; // another user be kicked ..
		
		std::string msg = command + " " + channel + " " + user + "\n";
		// sendClient(users[i].getSocket(), msg);
		for (std::size_t i = 0; i < users.size(); ++i) {
			if (users[i].getNickName() == parameters[1])
				sendClient(users[i].getSocket(), msg);
		}

		(void)iterUser;
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