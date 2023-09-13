#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#include <arpa/inet.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

const int SERVER_PORT = 6670;
const int MAX_CONNECTIONS = 10;

// // 예제에서 사용할 Capability 목록
// std::vector<std::string> supportedCapabilities = {"302", "other_capability"};

// // 함수: CAP LS 메시지 처리
// void HandleCAPLS(const std::string& clientMessage) {
//     // 클라이언트에서 보낸 CAP LS 메시지를 파싱하여 지원하는 Capability 목록 확인
//     // 예제에서는 supportedCapabilities에 미리 정의한 Capability 목록을 사용합니다.

//     // 클라이언트에게 지원하는 Capability 목록 응답
//     std::string response = "CAP LS :server supports=";

//     for (const std::string& capability : supportedCapabilities) {
//         response += capability + ",";
//     }

//     // 마지막 쉼표 제거
//     if (!supportedCapabilities.empty()) {
//         response.pop_back();
//     }

//     // 클라이언트에게 응답 전송
//     // send(clientSocket, response.c_str(), response.length(), 0);
// }

// // 함수: JOIN 메시지 처리
// void HandleJOIN(const std::string& clientMessage) {
//     // 클라이언트에서 보낸 JOIN 메시지를 파싱하여 채널 이름 확인
//     // 예제에서는 JOIN 메시지 형식을 간단하게 가정합니다.

//     // 채널 이름 추출 (예제에서는 ":" 뒤의 문자열을 채널 이름으로 가정)
//     std::string channelName = clientMessage.substr(6); // "JOIN :" 이후 문자열

//     // 채널에 클라이언트 입장 처리
//     std::cout << "Client joined channel: " << channelName << std::endl;

//     // 클라이언트에게 입장 확인 메시지 전송
//     // std::string response = "You have joined channel " + channelName;
//     // send(clientSocket, response.c_str(), response.length(), 0);
// }

class User
{
private:
	int socket;
	std::string nickName;
	std::string userName;
	std::string hostName;
	bool registered;

public:
	User(int socket) : socket(socket)
	{
	}

	int getSocket() const
	{
		return socket;
	}

	std::string getnickname() const
	{
		return nickName;
	}

	bool isRegistered()
	{
		return registered;
	}
};

class Channel
{
public:
	Channel(const std::string &name) : name(name) {}

	void addUser(const User &user)
	{
		users.push_back(user);
	}

	void removeUser(int socket)
	{
		users.erase(std::remove_if(users.begin(), users.end(),
								   [socket](const User &user)
								   {
									   return user.getSocket() == socket;
								   }),
					users.end());
	}

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

class IRCServer
{
private:
	int serverSocket;
	struct sockaddr_in serverAddress, clientAddr;
	std::vector<User> users;
	std::vector<Channel> channels;
	// struct pollfd pollfds[MAX_CONNECTIONS];
	std::vector<struct pollfd> pollfds;
	char buffer[532];

	// 사용자 추가
	void addUser(int clientSocket)
	{
		User user(clientSocket);
		users.push_back(user);
	}

	// 사용자 제거
	void removeUser(int clientSocket)
	{
		users.erase(std::remove_if(users.begin(), users.end(),
								   [clientSocket](const User &user)
								   {
									   return user.getSocket() == clientSocket;
								   }),
					users.end());

		// 채널에서도 해당 사용자 제거
		for (Channel &channel : channels)
		{
			channel.removeUser(clientSocket);
		}
	}

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

	// 사용자가 채널에 가입
	void joinChannel(int clientSocket, const std::string &channelName)
	{
		for (Channel &channel : channels)
		{
			if (channel.getName() == channelName)
			{
				channel.addUser(getUserBySocket(clientSocket));
				break;
			}
		}
	}

	// 사용자가 채널에서 나가기
	void leaveChannel(int clientSocket, const std::string &channelName)
	{
		for (Channel &channel : channels)
		{
			if (channel.getName() == channelName)
			{
				channel.removeUser(clientSocket);
				break;
			}
		}
	}

	// 소켓을 통해 사용자 찾기
	User getUserBySocket(int socket)
	{
		for (const User &user : users)
		{
			if (user.getSocket() == socket)
			{
				return user;
			}
		}
		throw std::runtime_error("User not found");
	}

	void handleClient(int clientSocket)
	{
		struct pollfd tmp;

		tmp.fd = clientSocket;
		tmp.events = POLLIN;
		std::string response;

		pollfds.push_back(tmp);

		User newUser(clientSocket);
		users.push_back(newUser);

		// while (1)
		//{
		// }

		response = "451 : client must be registered\n";
		send(clientSocket, response.c_str(), response.length(), 0);

		response = "001 yecnam :Welcome to the Internet Relay Network yecnam!yecnam@yecnam\n";
		send(clientSocket, response.c_str(), response.length(), 0);

		response = "002 :Your host is ft_irc, running version 1\n";
		send(clientSocket, response.c_str(), response.length(), 0);

		response = "003 :This server was created 2022.3.18\n";
		send(clientSocket, response.c_str(), response.length(), 0);

		response = "004 :ft_irc 1 +i +i\n";
		send(clientSocket, response.c_str(), response.length(), 0);

		response = "Mode yecnam +i\n";
		send(clientSocket, response.c_str(), response.length(), 0);

		std::cout << clientSocket << " : Client connected." << std::endl;
	}

public:
	IRCServer()
	{
		// 서버 초기화
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == -1)
		{
			std::cerr << "Error: Unable to create server socket." << std::endl;
			exit(1);
		}

		// 서버 주소 설정
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(SERVER_PORT);
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

		// for (size_t i = 1; i < MAX_CONNECTIONS; i++)
		// {
		//     pollfds[i].fd = -1;
		// }

		std::cout << "IRC Server started on port " << SERVER_PORT << std::endl;
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
			pollResult = poll(&pollfds[0], pollfds.size(), -1);
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

					std::cout << iter->fd << " client Received message: " << receivedMessage << std::endl;

					if (!iterUser->isRegistered())
					{
						if (receivedMessage == "JOIN :\n")
						{
							std::cout << "join\n";
							std::string response = "461";
							send(clientSocket, response.c_str(), response.length(), 0);
						}
					}
					else
					{
						if (receivedMessage == "JOIN :\n")
						{
							std::cout << "join\n";
							std::string response = "461";
							send(clientSocket, response.c_str(), response.length(), 0);
						}
						if (receivedMessage == "HI\n")
						{
							std::string response = "Hello, Client!\n";
							send(clientSocket, response.c_str(), response.length(), 0);
						}
					}
				}
			}
		}
	}
};

int main()
{
	IRCServer server;
	server.acceptConnections();
	return 0;
}
