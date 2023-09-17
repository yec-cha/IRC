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
	// std::string mode_; // ?


public:
	// void send_(int socket, const std::string& buffer, std::size_t size, int flags) {
	void send_(int socket, const std::string& buffer, int flags) {
		send(socket, buffer.c_str(), buffer.size(), flags);
	}

	// void sendAll_(int socket, const std::string& buffer, std::size_t size, int flags) {
	void sendAll_(const std::string& buffer, int flags) {
		for (std::map<int, User&>::const_iterator iter = users_.begin(); iter != this->users_.end(); ++iter) {
			send(iter->first, buffer.c_str(), buffer.size(), flags);
		}
	}

	/*	<message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
		<prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
		<command>  ::= <letter> { <letter> } | <number> <number> <number>
		<SPACE>    ::= ' ' { ' ' }
		<params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
		<middle>   ::= <Any *non-empty* sequence of octets not including SPACE
					or NUL or CR or LF, the first of which may not be ':'>
		<trailing> ::= <Any, possibly *empty*, sequence of octets not including
						NUL or CR or LF>
		<crlf>     ::= CR LF	*/
	std::string message_(const std::string& userNick, const std::string& ip, const std::string& command, const std::string& params) {
		std::string prefix = userNick + "!" + userNick + "@" + ip;
		std::string message = ":" + prefix + " " + command + " " + params + "\n";
		// std::string message = ":" + prefix + " " + command + params + "\n";
		return message;
	}

	// RPL_TOPIC (332) 
  	// "<client> <channel> :<topic>"
	std::string RPL_TOPIC(const std::string& client, const std::string& channel) {
		std::string command = "332";
		std::string result = ":" + command + client + channel + " :" + "\n";
		// std::string result = ":" + command + client + channel + " :" + this->getTopic() + "\n";
		return result;
	}

	// RPL_NAMREPLY (353) 
	// "<client> <symbol> <channel> :[prefix]<nick>{ [prefix]<nick>}"	
	// :irc.bnc.im 353 guest = #atheme :Rylee!rylai@localhost somasonic!andrew@somasonic.org
	std::string RPL_NAMREPLY(const std::string& client, const std::string& channel) {
		std::string command = "353";
		std::string symbol = "=";
		std::string result = ":" + command + client + symbol + channel + " :" + "\n";
		// std::string result = ":" + command + client + symbol + channel + " :" + clientList + "\n";
		return result;
	}

	// RPL_ENDOFNAMES (366)
  	// "<client> <channel> :End of /NAMES list"
	// :irc.bnc.im 366 guest #atheme :End of /NAMES list
	std::string RPL_ENDOFNAMES(const std::string& client, const std::string& channel) {
		std::string command = "366";
		std::string result = ":" + command + client + channel + ":End of /NAMES list\n"; 
		return result;
	}

	/*	20:18 -!- chanmukjung [~chanmukju@14.52.174.60] has joined #testroom
		20:18 [Users #testroom]
		20:18 [@chanmukjung]
		20:18 -!- Irssi: #testroom: Total of 1 nicks [1 ops, 0 halfops, 0 voices, 0 normal]	*/
	
	void welcomeChannel(std::vector<User>::iterator& iterUser) {
		std::string JOIN = ":" + iterUser->getNickName() + "!" + iterUser->getNickName() + "@" + "127.0.0.1" + " " + "JOIN" +  " " + this->name_ + "\n";
		this->sendAll_(JOIN, 0);

		// RPL_TOPIC (332) 
  		// "<client> <channel> :<topic>"
		std::string TOPIC_command = "332";
		std::string TOPIC = ": " + TOPIC_command + " " + iterUser->getNickName() + " " + this->name_ + " :TEST\n";
		// std::string TOPIC = ": " + TOPIC_command + " " + iterUser->getNickName() + " " + this->name_ + " :" + this->getTopic() + "\n";
		this->send_(iterUser->getSocket(), TOPIC, 0);

		// RPL_NAMREPLY (353) 
  		// "<client> <symbol> <channel> :[prefix]<nick>{ [prefix]<nick>}"
		// :irc.bnc.im 353 guest = #atheme :Rylee!rylai@localhost somasonic!andrew@somasonic.org
		std::string NAMREPLY_command = "353";
		std::string NAMREPLY_symbol = "=";
		std::string NAMREPLY = ": " + NAMREPLY_command + " " + iterUser->getNickName() + " " + NAMREPLY_symbol + " " + this->name_ + " :" + "\n";
		this->send_(iterUser->getSocket(), NAMREPLY, 0);

		// RPL_ENDOFNAMES (366) 
  		// "<client> <channel> :End of /NAMES list"
		// :irc.bnc.im 366 guest #atheme :End of /NAMES list
		std::string ENDOFNAMES_command = "366";
		std::string ENDOFNAMES = ": " + ENDOFNAMES_command + " " + iterUser->getNickName() + " " + this->name_ + " :End of /NAMES list\n";
		this->send_(iterUser->getSocket(), ENDOFNAMES, 0);
	}

	/*	<target>     ::= <to> [ "," <target> ]
		<to>         ::= <channel> | <user> '@' <servername> | <nick> | <mask>
		<channel>    ::= ('#' | '&') <chstring>
		<servername> ::= <host>
		<host>       ::= see RFC 952 [DNS:4] for details on allowed hostnames
		<nick>       ::= <letter> { <letter> | <number> | <special> }
		<mask>       ::= ('#' | '$') <chstring>
		<chstring>   ::= <any 8bit code except SPACE, BELL, NUL, CR, LF and comma (',')>
		<user>       ::= <nonwhite> { <nonwhite> }
		<letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
		<number>     ::= '0' ... '9'
		<special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'	*/



	// void welcomeChannel(std::vector<User>::iterator& iterUser) {
	// 	printInfo();
	// 	std::string join = ":" + iterUser->getNickName() + "!" + iterUser->getNickName() + "@" + "127.0.0.1" + " " + "JOIN" +  " " + this->name_ + "\n";
	// 	// send(4, join.c_str(), join.length(), 0);
	// 	this->sendAll_(join.c_str(), 0);
	// }

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
};

#endif