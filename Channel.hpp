#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <cstdlib>
# include <map>
# include <iostream>
# include <iterator>
# include <string>
# include <sys/socket.h>
# include <vector>
# include <deque>
# include <sstream>
# include <utility>

# include "User.hpp"

# define OPERATOR	1
# define NORMAL		0

class User;

class Channel
{
private:
	std::map<int, std::pair<int, User*> > users_;
	std::vector<std::string> invitedUser;
	std::string name_;
	std::string topicStr_;
	std::string password_;

	bool invite_;
	bool key_;
	bool topic_;
	int userLimit_;

	Channel();

public:
	Channel(const std::string &name, User &user);
	Channel(const Channel &channel);
	Channel& operator=(const Channel &channel);
	~Channel();

	void addUser(User &user);
	void delUser(User &user);
	void deleteUser(const int socket);
	void addInvitedUser(const std::string &name);
	void setTopic(const std::string &topic);
	void setPass(const std::string &pass);

	const std::string &getPassword() const;
	const std::string &getName() const;
	const std::string &getTopic() const;
	int getLimit() const;
	bool getInviteBool() const;
	bool getKeyBool() const;
	bool getIsTopic() const;
	int getSize() const;

	void welcomeChannel(User &user);

	bool isInvited(User &user);
	bool isInChannel(User &user) const;
	bool isInChannel(const std::string& user) const;
	bool isOperator(User &user) const;
	
	void send_(int socket, const std::string &buffer, int flags);
	void sendAll_(const std::string &buffer, int flags);
	void sendPrivMsg_(const std::string &buffer, int socket, int flags);
	void sendChannelPRIVMSG(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	
	const std::string MODE(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	const std::string NOTICE(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void inviteMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void topicMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void keyMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void operatorMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void userLimitMode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
	void mode(const std::vector<std::string> &parameters, std::deque<User>::iterator &iterUser);
};

#endif