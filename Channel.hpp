#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>

class Channel
{
public:
	Channel(const std::string &name) : name(name) {}

	void addUser(const User &user)
	{
		users.push_back(user);
	}

	// void removeUser(int socket)
	// {
	// 	users.erase(std::remove_if(users.begin(), users.end(),
	// 							   [socket](const User &user)
	// 							   {
	// 								   return user.getSocket() == socket;
	// 							   }),
	// 				users.end());
	// }

	std::string getName() const
	{
		return name;
	}

	const std::vector<User> &getUsers() const
	{
		return users;
	}

private:
	std::string name;
	std::vector<User> users;
};

#endif