#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <map>
# include <iostream>
# include <iterator>
# include <string>
# include <sys/socket.h>
# include <vector>
# include <deque>

enum ChannelType{
	PUBLIC,
	SECRET,
	PRIVATE
};

class Channel {
private:
	std::string name_;
	std::string topic_;
	std::string password_;
	std::multimap<int, User&> users_; // operator, userInfo
	std::string mode_;
	int channelType;

public:
	int getType() const
	{
		return channelType;
	}
	// void send_(int socket, const std::string& buffer, std::size_t size, int flags) {
	void send_(int socket, const std::string& buffer, int flags) {
		std::cout << BLUE "server to (" << socket << ") : " << buffer << RESET << std::endl;
		send(socket, buffer.c_str(), buffer.size(), flags);
	}

	// void sendAll_(int socket, const std::string& buffer, std::size_t size, int flags) {
	void sendAll_(const std::string& buffer, int flags) {
		for (std::multimap<int, User&>::const_iterator iter = users_.begin(); iter != this->users_.end(); ++iter) {
			send(iter->second.getSocket(), buffer.c_str(), buffer.size(), flags);
		}
	}

	std::string message_(const std::string& userNick, const std::string& ip, const std::string& command, const std::string& params) {
		std::string prefix = userNick + "!" + userNick + "@" + ip;
		std::string message = ":" + prefix + " " + command + " " + params + "\n";
		// std::string message = ":" + prefix + " " + command + params + "\n";
		return message;
	}

	std::string RPL_TOPIC(const std::string& client, const std::string& channel) {
		std::string command = "332";
		std::string result = ":" + command + client + channel + " :" + "\n";
		// std::string result = ":" + command + client + channel + " :" + this->getTopic() + "\n";
		return result;
	}

	std::string RPL_NAMREPLY(const std::string& client, const std::string& channel) {
		std::string command = "353";
		std::string symbol = "=";
		std::string result = ":" + command + client + symbol + channel + " :" + "\n";
		// std::string result = ":" + command + client + symbol + channel + " :" + clientList + "\n";
		return result;
	}

	std::string RPL_ENDOFNAMES(const std::string& client, const std::string& channel) {
		std::string command = "366";
		std::string result = ":" + command + client + channel + ":End of /NAMES list\n"; 
		return result;
	}

	void welcomeChannel(std::deque<User>::iterator& iterUser) {
		std::string JOIN = ":" + iterUser->getNickName() + "!" + iterUser->getNickName() + "@" + iterUser->getHostName() + " " + "JOIN" +  " " + this->name_ + "\n";
		this->sendAll_(JOIN, 0);

		std::string TOPIC_command = "332";
		std::string TOPIC = ":ft_IRC " + TOPIC_command + " " + iterUser->getNickName() + " " + this->name_ + " :TEST\n";
		// std::string TOPIC = ": " + TOPIC_command + " " + iterUser->getNickName() + " " + this->name_ + " :" + this->getTopic() + "\n";
		this->send_(iterUser->getSocket(), TOPIC, 0);

		std::string NAMREPLY_command = "353";
		std::string NAMREPLY_symbol = "=";
		std::string NAMREPLY = ":ft_IRC " + NAMREPLY_command + " " + iterUser->getNickName() + " " + NAMREPLY_symbol + " " + this->name_ + " :";
		
		std::string nameOfClient;
		for (std::multimap<int, User&>::iterator it = users_.begin(); it != users_.end(); it++)
		{
			if (it->first)
				nameOfClient = "@" + it->second.getNickName() + " ";
			else
				nameOfClient = it->second.getNickName() + " ";
			NAMREPLY += nameOfClient;
		}

		NAMREPLY += "\n";
		this->send_(iterUser->getSocket(), NAMREPLY, 0);
		// this->sendAll_(NAMREPLY, 0);

		std::string ENDOFNAMES_command = "366";
		std::string ENDOFNAMES = ":ft_IRC " + ENDOFNAMES_command + " " + iterUser->getNickName() + " " + this->name_ + " :End of /NAMES list\n";
		this->send_(iterUser->getSocket(), ENDOFNAMES, 0);
		// this->sendAll_(ENDOFNAMES, 0);
	}

	void sendPRIVMSG(const std::vector<std::string>& parameters, std::deque<User>::iterator& iterUser) {
		for (std::multimap<int, User&>::iterator iter = this->users_.begin(); iter != this->users_.end(); ++iter) {
			if (iterUser->getSocket() != iter->second.getSocket()) {
				std::string privmsg = ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + "127.0.0.1" + " " + "PRIVMSG " + parameters[0] + " :" + parameters[1] + "\n";
				this->send_(iter->second.getSocket(), privmsg, 0);
			}
		}
	}


	/*
	- i: Set/remove Invite-only channel
		초대 전용 채널 설정/제거
	- t: Set/remove the restrictions of the TOPIC command to channel operators
		채널에 대한 TOPIC 명령의 제한을 설정/제거
	- k: Set/remove the channel key (password)
		채널 키(비밀번호) 설정/제거
	- o: Give/take channel operator privilege
		채널 운영자 권한 부여/수여
	*/
	// Command: MODE
	// Parameters: <target> [<modestring> [<mode arguments>...]]
	//void mode(const std::vector<std::string>& parameters, std::deque<User>::iterator& iterUser) { {
	//	/*
	//		/mode +o NYC
	//		params[0]: +o
	//		params[1]: NYC
	//	*/
	//	// error handling
	//	if (parameters[0].size() != 2) {
	//		std::cout << "The size of parameters[0] is not 2." << std::endl;
	//		return ;
	//	}
	//	if (parameters[0].at(1))
	//}
	

	// //KICK 명령어: 클라이언트를 채널에서 강퇴
	// void kick(struct Client *operator, struct Client *target) {
	// 	if (operator->isOperator) {
	// 		// 채널 운영자만이 클라이언트를 강퇴할 수 있음
	// 		printf("%s kicked %s from the channel.\n", operator->nick, target->nick);
	// 	} else {
	// 		printf("You do not have permission to kick users.\n");
	// 	}
	// }

	// // INVITE 명령어: 클라이언트를 채널에 초대
	// void invite(struct Client *operator, struct Client *target) {
	// 	if (operator->isOperator) {
	// 		// 채널 운영자만이 클라이언트를 채널에 초대할 수 있음
	// 		printf("%s invited %s to the channel.\n", operator->nick, target->nick);
	// 	} else {
	// 		printf("You do not have permission to invite users.\n");
	// 	}
	// }

	// //TOPIC 명령어: 채널 주제 변경 또는 보기
	// void topic(struct Client *operator, const char *newTopic) {
	// 	if (operator->isOperator) {
	// 		// 채널 운영자만이 채널 주제를 변경할 수 있음
	// 		strcpy(channel.topic, newTopic);
	// 		printf("%s changed the channel topic to: %s\n", operator->nick, channel.topic);
	// 	} else {
	// 		// 채널 주제를 보기만 할 수 있음
	// 		printf("Channel topic: %s\n", channel.topic);
	// 	}
	// }

	bool isOperator(User& user) const {
		std::multimap<int, User&>::const_iterator iter = this->users_.begin();
		for (; iter != this->users_.end(); ++iter) {
			if (iter->second.getSocket() == user.getSocket() && iter->first == 1)
				return true;
		}
		return false;
	}

	// MODE 명령어: 채널 모드 변경
	// void mode(struct Client *operator, char mode, const char *parameter) {
	void mode(const std::vector<std::string>& parameters, std::deque<User>::iterator& iterUser) {
		// std::cout << "MODE " << std::endl;
		// std::cout << "parameters[0]: " << parameters[0] << std::endl;
		// std::cout << "parameters[1]: " << parameters[1] << std::endl;
		// std::cout << "MODE " << std::endl;
		// if (isOperator(*iterUser) == true) {
		
		(void)iterUser;
		// ex) /mode #4444 +i a
		// MODE #4444 +ia
		// parameters[0]: #4444
		// parameters[1]: +ia

		switch (parameters[1].at(1)) {
			case 'i':
				std::cout << "mode parameter is " << parameters[1].at(1) << std::endl;
				// channel.isInviteOnly = !channel.isInviteOnly;
				// printf("Channel is now %sinvite-only.\n", channel.isInviteOnly ? "" : "not ");
				break;
			case 't':
				std::cout << "mode parameter is " << parameters[1].at(1) << std::endl;
				// 채널 주제에 대한 TOPIC 명령의 제한을 설정/제거
				// 이 부분은 실제 채팅 서버에 맞게 구현되어야 함
				break;
			case 'k':
				std::cout << "mode parameter is " << parameters[1].at(1) << std::endl;
				// strcpy(channel.channelKey, parameter);
				// printf("Channel key is set to: %s\n", channel.channelKey);
				break;
			case 'o':
				std::cout << "mode parameter is " << parameters[1].at(1) << std::endl;
				// 클라이언트에게 채널 운영자 권한 부여/박탈
				// 이 부분은 실제 채팅 서버에 맞게 구현되어야 함
				break;
			default:
				std::cout << "TEST" << std::endl;
				// printf("Invalid mode: %c\n", mode);
				break;
		}
		// }
		// else
		// 	// There is no permission
		// 	// ERR_CHANOPRIVSNEEDED (482)
		// 	// std::cout << "You do not have permission to change the channel mode." << std::endl;
	}


	Channel(const std::string &name, User& user) : name_(name) {
		channelType = PUBLIC;
		addUser(user);
	}

	void addUser(User& user) {
		this->users_.insert(std::pair<bool, User&>(true, user));
	}

	void delUser(User& user) {
		this->users_.erase(user.getSocket());
	}

	void setTopic(const std::string& topic) {
		this->topic_ = topic;
	}

	void setPass(const std::string& pass) {
		this->topic_ = pass;
	}

	const std::string& getName() const {
		return this->name_;
	}

	const std::string& getTopic() const {
		return this->topic_;
	}
	
	const std::string& getPassword() const {
		return this->password_;
	}

	int getSize() const {
		return this->users_.size();
	}

	void printInfo() const {
		std::cout << "[CHANNEL INFO]" << std::endl;
		std::cout << "NAME: " << this->name_ << std::endl;
		std::cout << "TOPIC: " << this->topic_ << std::endl;
		std::cout << "PASS: " << this->password_ << std::endl;
		
		std::cout << " - LIST" << std::endl;
		for (std::multimap<int, User&>::const_iterator iter = users_.begin(); iter != this->users_.end(); ++iter) {
			std::cout << "FD: " << iter->second.getSocket() << ", USER NAME: " << iter->second.getUserName() << std::endl;
		}
		std::cout << " -" << std::endl;
		std::cout << "[CHANNEL INFO]" << std::endl;
	}
};

#endif