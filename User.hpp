#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <map>
#include "color.hpp"
#include <iostream>

/* Parameters: <username> <hostname> <servername> <realname> */
class User
{
private:
	int socket_;

	std::string nickName_;
	std::string userName_;
	std::string hostName_;
	std::string serverName_;
	std::string realName_;

	bool isPassed;
	bool hasNick;
	bool hasUser;
	bool isRegistered;
	bool isEnd;

	User();

public:
	User(int socket);
	User(const User &user);
	User &operator=(const User &user);
	~User();

	int getSocket();
	const std::string &getNickName() const;
	const std::string &getUserName() const;
	const std::string &getHostName() const;
	const std::string &getServerName() const;
	const std::string &getRealName() const;

	bool getIsPassed() const;
	bool getHasNick() const;
	bool getHasUser() const;
	bool getIsRegistered() const;
	bool getIsEnd() const;

	void setIsPassed(bool flag);
	void setHasNick(bool flag);
	void setHasUser(bool flag);
	void setIsRegistered(bool flag);
	void endCilent();

	void setNickName(const std::string &name);
	void setUserName(const std::string &name);
	void setHostName(const std::string &name);
	void setServerName(const std::string &name);
	void setRealName(const std::string &name);

	void getMyState() const
	{
		std::cout << YELLOW << socket_ << " : " << nickName_ << " registerd ? " << isRegistered << " end ? " << isEnd << RESET << std::endl;
	}
};

#endif