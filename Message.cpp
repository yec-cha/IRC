#include "Messege.hpp"

Message::Message() {}

Message::Message(const Message &message)
{
	*this = message;
}

Message& Message::operator=(const Message &message)
{
	if (this != &message)
		std::cout << "This function is not used." << std::endl;
	return *this;
}

Message::~Message() {}

void Message::sendClient(int socket, const std::string &msg)
{
	send(socket, msg.c_str(), msg.length(), 0);
	std::cout << "server (to " << socket << ") :" << RED << msg << RESET << std::endl;
};

void Message::RPL_CHANNELMODEIS_324(int socket, const std::string &client, const std::string &channel)
{
	const std::string response = "324 " + client + " " + channel + "\n";
	sendClient(socket, response);
};

void Message::RPL_NOTOPIC_331(int socket, const std::string& client, const std::string& channel)
{
	const std::string response = "331 " + client + " " + channel + " :No topic is set\n";
	sendClient(socket, response);
};

void Message::RPL_TOPIC_332(int socket, const std::string& client, const std::string &channel, const std::string &topic)
{
	const std::string response = "332 " + client + " " + channel + " :" + topic + "\n";
	sendClient(socket, response);
};

void Message::ERR_NOSUCHNICK_401(int socket, const std::string& client, const std::string& nickname)
{
	const std::string response = "401 " + client + " " + nickname + " :No such nick/channel\n";
	sendClient(socket, response);
};

void Message::ERR_NOSUCHCHANNEL_403(int socket, const std::string &client, const std::string &channel)
{
	const std::string response = "403 " + client + " " + channel + " :No such channel\n";
	sendClient(socket, response);
};

void Message::ERR_NOORIGIN_409(int socket, const std::string& client)
{
	const std::string response = "409 " + client + " :No origin specified\n";
	sendClient(socket, response);
};

void Message::ERR_UNKNOWNCOMMAND_421(int socket, const std::string& client, const std::string &cmd)
{
	const std::string response = "421 "  + client + " " + cmd + " :Unknown command\n";
	sendClient(socket, response);
};

void Message::ERR_NONICKNAMEGIVEN_431(int socket, const std::string& client)
{
	const std::string response = "431 " + client + " :No nickname given\n";
	sendClient(socket, response);
};

void Message::ERR_ERRONEUSNICKNAME_432(int socket, const std::string& client, const std::string &nick)
{
	const std::string response = "432 " + client + " " + nick + " :Erroneus nickname\n";
	sendClient(socket, response);
};

void Message::ERR_NICKNAMEINUSE_433(int socket, const std::string &nick, const std::string &newNick)
{
	std::string response;
	if (nick.size() == 0 || nick == "")
		response = "433 anonymous " + newNick + " :Nickname is already in use\n";
	else
		response = "433 " + nick + " " + newNick + " :Nickname is already in use\n";
	sendClient(socket, response);
};

void Message::ERR_USERNOTINCHANNEL_441(int socket, const std::string &client, const std::string& user, const std::string &channel)
{
	const std::string response = "441 " + client + " " + user + " " + channel + " :They aren't on that channell\n";
	sendClient(socket, response);
};

void Message::ERR_NOTONCHANNEL_442(int socket, const std::string &client, const std::string &channel)
{
	const std::string response = "442 " + client + " " + channel + " :You're not on that channel\n";
	sendClient(socket, response);
};

void Message::ERR_USERONCHANNEL_443(int socket, const std::string &client, const std::string& user, const std::string &channel)
{
	const std::string response = "443 " + client + " " + user + " " + channel + " :is already on channel\n";
	sendClient(socket, response);
};

void Message::ERR_NOTREGISTERED_451(int socket, const std::string& client)
{
	const std::string response = "451 " + client + " :You have not registered\n";
	sendClient(socket, response);
};

void Message::ERR_NEEDMOREPARAMS_461(int socket, const std::string &client, const std::string &command)
{
	const std::string response = "461 " + client + " " + command + " :Not enough parameters\n";
	sendClient(socket, response);
};

void Message::ERR_ALREADYREGISTERED_462(int socket, const std::string& client)
{
	const std::string response = "462 " + client + " :You may not reregister\n";
	sendClient(socket, response);
};

void Message::ERR_PASSWDMISMATCH_464(int socket, const std::string& client)
{
	const std::string response = "464 " + client + " :Password incorrect\n";
	sendClient(socket, response);
};

void Message::ERR_CHANNELISFULL_471(int socket, const std::string &nickName, const std::string &channelName)
{
	const std::string response = "471 " + nickName + " " + channelName + " :Cannot join channel (+l)\n";
	sendClient(socket, response);
};

void Message::ERR_INVITEONLYCHAN_473(int socket, const std::string &nickName, const std::string &channelName)
{
	const std::string response = "473 " + nickName + " " + channelName + " :Cannot join channel (+i)\n";
	sendClient(socket, response);
};

void Message::ERR_BADCHANNELKEY_475(int socket, const std::string &nickName, const std::string &channelName)
{
	const std::string response = "475 " + nickName + " " + channelName + " :Cannot join channel (+k)\n";
	sendClient(socket, response);
};

void Message::ERR_CHANOPRIVSNEEDED_482(int socket, const std::string &client, const std::string &channel)
{
	const std::string response = "482 " + client + " " + channel + " :You're not channel operator\n";
	sendClient(socket, response);
};