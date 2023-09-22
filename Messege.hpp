#include <iostream>
# include <string>

# include <iostream>
# include <vector>
# include <string>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <poll.h>

# include "color.hpp"

class Message
{
private:
	Message();
	Message(const Message &message);
	Message& operator=(const Message &message);
	~Message();

	static void sendClient(int socket, const std::string &msg);

public:
	static void RPL_CHANNELMODEIS_324(int socket, const std::string &client, const std::string &channel);
	static void RPL_NOTOPIC_331(int socket, const std::string& client, const std::string& channel);	
	static void RPL_TOPIC_332(int socket, const std::string& client, const std::string &channel, const std::string &topic);
	static void ERR_NOSUCHNICK_401(int socket, const std::string& client, const std::string& nickname);	
	static void ERR_NOSUCHCHANNEL_403(int socket, const std::string &client, const std::string &channel);
	static void ERR_NOORIGIN_409(int socket, const std::string& client);
	static void ERR_UNKNOWNCOMMAND_421(int socket, const std::string& client, const std::string &cmd);
	static void ERR_NONICKNAMEGIVEN_431(int socket, const std::string& client);
	static void ERR_ERRONEUSNICKNAME_432(int socket, const std::string& client, const std::string &nick);
	static void ERR_NICKNAMEINUSE_433(int socket, const std::string &nick, const std::string &newNick);
	static void ERR_USERNOTINCHANNEL_441(int socket, const std::string &client, const std::string& user, const std::string &channel);
	static void ERR_NOTONCHANNEL_442(int socket, const std::string &client, const std::string &channel);
	static void ERR_USERONCHANNEL_443(int socket, const std::string &client, const std::string& user, const std::string &channel);
	static void ERR_NOTREGISTERED_451(int socket, const std::string& client);
	static void ERR_NEEDMOREPARAMS_461(int socket, const std::string &client, const std::string &command);
	static void ERR_ALREADYREGISTERED_462(int socket, const std::string& client);
	static void ERR_PASSWDMISMATCH_464(int socket, const std::string& client);
	static void ERR_CHANNELISFULL_471(int socket, const std::string &nickName, const std::string &channelName);
	static void ERR_INVITEONLYCHAN_473(int socket, const std::string &nickName, const std::string &channelName);
	static void ERR_BADCHANNELKEY_475(int socket, const std::string &nickName, const std::string &channelName);
	static void ERR_CHANOPRIVSNEEDED_482(int socket, const std::string &client, const std::string &channel);
};
