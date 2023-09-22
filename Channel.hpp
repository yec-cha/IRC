#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <map>
# include <iostream>
# include <iterator>
# include <string>
# include <sys/socket.h>
# include <vector>
# include <deque>
# include <sstream>

# include "User.hpp"

# define OPERATOR	1
# define NORMAL		0

class User;

class Channel
{
private:
	std::map<int, std::pair<int, User&> > users_; // socket, <operator, userInfo>
	std::vector<std::string> invitedUser;
	std::string name_;
	std::string topicStr_;
	std::string password_;
	

	bool invite_;
	bool key_;
	bool topic_;
	int userLimit_;

public:
	void addInvitedUser(const std::string &name);

	bool isInvited(User &user);
	int getLimit() const;
	bool getInviteBool() const;
	bool getKeyBool() const;
	bool getIsTopic() const;
	const std::string &getPassword() const;

	void send_(int socket, const std::string &buffer, int flags);
	void sendAll_(const std::string &buffer, int flags);
	void sendPrivMsg_(const std::string &buffer, int socket, int flags);
	void sendChannelPRIVMSG(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	std::string message_(const std::string &userNick, const std::string &ip, const std::string &command, const std::string &params);
	std::string RPL_TOPIC(const std::string &client, const std::string &channel);
	std::string RPL_NAMREPLY(const std::string &client, const std::string &channel);
	std::string RPL_ENDOFNAMES(const std::string &client, const std::string &channel);
	void welcomeChannel(User &user);

	const std::string MODE(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	const std::string NOTICE(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void inviteMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void topicMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void keyMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void operatorMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void userLimitMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	bool isInChannel(User &user) const;
	bool isInChannel(const std::string& user) const;
	bool isOperator(User &user) const;
	void mode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);

	Channel(const std::string &name, User &user);
	~Channel();

	void addUser(User &user);
	void delUser(User &user);
	void setTopic(const std::string &topic);
	void setPass(const std::string &pass);
	const std::string &getName() const;
	const std::string &getTopic() const;
	int getSize() const;
	void deleteUser(const int socket);
};

#endif