#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <map>
# include <iostream>
# include <iterator>
# include <string>
# include <sys/socket.h>
# include <vector>

class Channel {
private:
	std::string name_;
	std::string topic_;
	std::string password_;
	std::map<int, User&> users_;
	int operator_;
	// int mode_;

public:
	void printInfo() const {
		std::cout << "[CHANNEL INFO]" << std::endl;
		std::cout << "NAME: " << this->name_ << std::endl;
		std::cout << "TOPIC: " << this->topic_ << std::endl;
		std::cout << "PASS: " << this->password_ << std::endl;
		
		std::cout << " - LIST" << std::endl;
		for (std::map<int, User&>::const_iterator iter = users_.begin(); iter != this->users_.end(); ++iter) {
			std::cout << "FD: " << iter->first << ", USER NAME: " << iter->second.getUserName() << std::endl;
		}
		std::cout << " -" << std::endl;
		std::cout << "[CHANNEL INFO]" << std::endl;
	}

	/*
	<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
	<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
	<command>  ::= <letter> { <letter> } | <number> <number> <number>
	<SPACE>    ::= ' ' { ' ' }
	<params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
	<middle>   ::= <Any *non-empty* sequence of octets not including SPACE
				or NUL or CR or LF, the first of which may not be ':'>
	<trailing> ::= <Any, possibly *empty*, sequence of octets not including
					NUL or CR or LF>
	<crlf>     ::= CR LF
	*/
	void sendPRIVMSG(const std::vector<std::string>& parameters, std::vector<User>::iterator& iterUser) {
		(void)iterUser;
		// std::map<int, User&>::iterator first_ = users_.begin();
		// std::map<int, User&>::iterator last_ = users_.end();
		// for (; first_ != last_ < ++first_) {
		// 	send(first_->first, parameters[1].c_str(), parameters[1].length(), 0);
		// }
		// std::string messages;
		this->printInfo();
		// for (std::map<int, User&>::iterator iter = this->users_.begin(); iter != this->users_.end(); ++iter) {
		// 	// std::string messages = parameters[1];
		// 	send(iter->first, parameters[1].c_str(), parameters[1].length(), 0);
		// }
		for (std::map<int, User&>::iterator iter = this->users_.begin(); iter != this->users_.end(); ++iter) {
			std::string privmsg = ":" + iterUser->getNickName() + "!" + iterUser->getUserName() + "@" + "127.0.0.1" + " " + "PRIVMSG" + " " + parameters[0] + " " + parameters[1] + "\n";
			send(iter->first, privmsg.c_str(), privmsg.length(), 0);
		}
	}

	/*
	20:18 -!- chanmukjung [~chanmukju@14.52.174.60] has joined #testroom
	20:18 [Users #testroom]
	20:18 [@chanmukjung]
	20:18 -!- Irssi: #testroom: Total of 1 nicks [1 ops, 0 halfops, 0 voices, 0 normal]
	*/
	void welcomeChannel(std::vector<User>::iterator& iterUser) {
		// printInfo();
		std::string str = ":" + iterUser->getNickName() + "!" + iterUser->getNickName() + "@" + "127.0.0.1" + " " + "JOIN" +  " " + this->name_ + "\n";
		send(4, str.c_str(), str.length(), 0);
	}

	Channel(const std::string &name, User& user) : name_(name) {
		addUser(user);
		setOperator(user.getSocket());
	}

	void addUser(User& user) {
		this->users_.insert(std::pair<int, User&>(user.getSocket(), user));
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

	void setOperator(int socket) {
		this->operator_ = socket;
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

	// bool checkOperator(const User& user) const {
	bool checkOperator(User& user) const {
		if (this->operator_ == user.getSocket())
			return true;
		return false;
	}
};

#endif