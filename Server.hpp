#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <string>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <poll.h>

# include "User.hpp"
# include "Channel.hpp"

const int MAX_CONNECTIONS = 10;

class User;
class Channel;

class IRCServer {
private:
	int serverSocket;
	struct sockaddr_in serverAddress, clientAddr;
	std::vector<User> users;
	std::vector<Channel> channels;
	// struct pollfd pollfds[MAX_CONNECTIONS];
	std::vector<struct pollfd> pollfds;
	char buffer[532];
    int port_;

	void cmdNick(std::vector<User>::iterator &iter, std::string &msg)
	{
		std::string response;

		if (iter->getIsRegistered())
		{
			if (iter->getIsPassed())
			{
				iter->setNickName(msg);
			}
		}
		else
		{
			response = ":yecnam NICK hi\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}
	}

	void beforeRegisterdMsg(std::string &cmd, std::string &msg, std::vector<User>::iterator &iter)
	{
		std::string response;

		if (cmd == "NICK")
		{
			cmdNick(iter, msg);
		}
		else if (cmd == "PASS")
		{
			iter->setIsPassed(true);
		}
		else if (cmd == "USER")
		{
			iter->setIsRegistered(true);
		}
		else if (cmd == "CAP")
		{
			;
		}
		else
		{
			response = "451 : client must be registered\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}

		if (iter->getIsRegistered())
		{
			response = "001 yecnam :Welcome to the Internet Relay Network yecnam!yecnam@yecnam\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "002 :Your host is ft_irc, running version 1\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "003 :This server was created 2022.3.18\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "004 :ft_irc 1 +i +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);

			response = "Mode yecnam +i\n";
			send(iter->getSocket(), response.c_str(), response.length(), 0);
		}
	}

	// 사용자 추가
	void addUser(int clientSocket)
	{
		User user(clientSocket);
		users.push_back(user);
	}

	// 사용자 제거
	// void removeUser(int clientSocket)
	// {
	// 	users.erase(std::remove_if(users.begin(), users.end(),
	// 							   [clientSocket](const User &user)
	// 							   {
	// 								   return user.getSocket() == clientSocket;
	// 							   }),
	// 				users.end());

	// 	// 채널에서도 해당 사용자 제거
	// 	for (Channel &channel : channels)
	// 	{
	// 		channel.removeUser(clientSocket);
	// 	}
	// }

	// 채널 추가
	void addChannel(const std::string &channelName)
	{
		Channel channel(channelName);
		channels.push_back(channel);
	}

	// 채널 제거
	void removeChannel(const std::string &channelName)
	{
		channels.erase(std::remove_if(channels.begin(), channels.end(),
									  [channelName](const Channel &channel)
									  {
										  return channel.getName() == channelName;
									  }),
					   channels.end());
	}

	// // 사용자가 채널에 가입
	// void joinChannel(int clientSocket, const std::string &channelName)
	// {
	// 	for (Channel &channel : channels)
	// 	{
	// 		if (channel.getName() == channelName)
	// 		{
	// 			channel.addUser(getUserBySocket(clientSocket));
	// 			break;
	// 		}
	// 	}
	// }

	// 사용자가 채널에서 나가기
	// void leaveChannel(int clientSocket, const std::string &channelName)
	// {
	// 	for (Channel &channel : channels)
	// 	{
	// 		if (channel.getName() == channelName)
	// 		{
	// 			channel.removeUser(clientSocket);
	// 			break;
	// 		}
	// 	}
	// }

	// // 소켓을 통해 사용자 찾기
	// User getUserBySocket(int socket)
	// {
	// 	for (const User &user : users)
	// 	{
	// 		if (user.getSocket() == socket)
	// 		{
	// 			return user;
	// 		}
	// 	}
	// 	throw std::runtime_error("User not found");
	// }

	void handleClient(int clientSocket)
	{
		struct pollfd tmp;

		tmp.fd = clientSocket;
		tmp.events = POLLIN;

		pollfds.push_back(tmp);

		User newUser(clientSocket);
		users.push_back(newUser);

		std::cout << clientSocket << " : Client connected." << std::endl;
	}

public:
	IRCServer() {}
    IRCServer(int port) : port_(port) {
		// 서버 초기화
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == -1)
		{
			std::cerr << "Error: Unable to create server socket." << std::endl;
			exit(1);
		}

		// 서버 주소 설정
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(this->port_);
		serverAddress.sin_addr.s_addr = INADDR_ANY;

		// 서버 바인딩
		if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
		{
			std::cerr << "Error: Binding failed." << std::endl;
			exit(1);
		}

		// 서버 리스닝
		if (listen(serverSocket, MAX_CONNECTIONS) == -1)
		{
			std::cerr << "Error: Listening failed." << std::endl;
			exit(1);
		}

		struct pollfd tmp;
		tmp.fd = serverSocket;
		tmp.events = POLLIN;

		pollfds.push_back(tmp);

		std::cout << "IRC Server started on port " << this->port_ << std::endl;
	}

	void acceptConnections()
	{
		int clientLen = sizeof(clientAddr);
		int clientSocket;
		int pollResult;
		std::vector<User>::iterator iterUser;
		std::vector<struct pollfd>::iterator iter;

		while (true)
		{
			pollResult = poll(&pollfds[0], pollfds.size(), 0);
			if (pollResult == -1)
			{
				// 오류 처리
				std::cerr << "Error in poll." << std::endl;
				break;
			}
			else if (pollResult == 0)
			{
				// 타임아웃 처리
				continue;
			}

			if (pollfds[0].revents & POLLIN)
			{
				clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&clientLen);
				if (clientSocket == -1)
				{
					std::cerr << "Error: Unable to accept client connection." << std::endl;
					continue;
				}
				handleClient(clientSocket);
			}

			for (iter = pollfds.begin() + 1, iterUser = users.begin(); (iter != pollfds.end()) && (iterUser != users.end()); iter++, iterUser++)
			{
				if (iter->fd > 0 && iter->revents & POLLIN)
				{
					ssize_t bytesRead = recv(iter->fd, buffer, sizeof(buffer), 0);
					if (bytesRead <= 0)
					{
						std::cout << iter->fd << ": Client disconnected." << std::endl;
						close(iter->fd);
						pollfds.erase(iter);
						users.erase(iterUser);
						break;
					}

					// 수신한 데이터를 문자열로 변환
					std::string receivedMessage(buffer, bytesRead);
					std::string oneMsg;
					std::stringstream ss(receivedMessage);
					std::string command;
					std::cout << iter->fd << " client Received message: " << receivedMessage << std::endl;

					while (std::getline(ss, oneMsg))
					{
						command = oneMsg.substr(0, oneMsg.find(" "));
						std::cout << "command : " << command << std::endl;

						if (!iterUser->getIsRegistered())
						{
							beforeRegisterdMsg(command, oneMsg, iterUser);
						}
						else
						{
							if (command == "PING")
							{
								// std::string response = "PONG " + iterUser->getnickname();
                                std::string response = "PONG localhost\n";
								send(clientSocket, response.c_str(), response.length(), 0);
							}
							if (command == "HI\n")
							{
								std::string response = "Hello, Client!\n";
								send(clientSocket, response.c_str(), response.length(), 0);
							}
						}
					}
				}
			}
		}
	}
};


#endif