#ifndef USER_HPP
# define USER_HPP

# include <string>

class User
{
private:
	int socket;
	std::string nickName;
	std::string userName;
	std::string hostName;

public:
	bool hasNick;
	bool hasUser;
	bool isPassed;
	bool isRegistered;

	User(int socket) : socket(socket)
	{
		isRegistered = false;
		isPassed = false;
		hasNick = false;
		hasUser = false;
	}

	int getSocket() const
	{
		return socket;
	}

	std::string getnickname() const
	{
		return nickName;
	}

	void setNick(const std::string &nick)
	{
		nickName = nick;
	}
};

#endif