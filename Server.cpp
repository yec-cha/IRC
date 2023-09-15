#include "Server.hpp"

IRCServer::IRCServer() : cm(users, channels) {}

IRCServer::IRCServer(int port) : cm(users, channels), port_(port)
{
	this->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverSocket == -1)
	{
		std::cerr << "Error: Unable to create server socket." << std::endl;
		exit(1);
	}

	this->serverAddress.sin_family = AF_INET;
	this->serverAddress.sin_port = htons(this->port_);
	this->serverAddress.sin_addr.s_addr = INADDR_ANY;
	if (bind(serverSocket, (struct sockaddr *)&this->serverAddress, sizeof(this->serverAddress)) == -1)
	{
		std::cerr << "Error: Binding failed." << std::endl;
		exit(1);
	}

	if (listen(this->serverSocket, MAX_CONNECTIONS) == -1)
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

void IRCServer::cmdNick(std::vector<User>::iterator &iter, std::string &msg)
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

void IRCServer::beforeRegisterdMsg(std::string &cmd, std::string &msg, std::vector<User>::iterator &iter)
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

void IRCServer::handleClient(int clientSocket)
{
	struct pollfd tmp;

	tmp.fd = clientSocket;
	tmp.events = POLLIN;

	pollfds.push_back(tmp);

	User newUser(clientSocket);
	users.push_back(newUser);

	std::cout << clientSocket << " : Client connected." << std::endl;
}

void IRCServer::acceptConnections()
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
			memset(buffer, 0, 532);
			if (iter->fd > 0 && iter->revents & POLLIN)
			{
				ssize_t bytesRead = recv(iter->fd, buffer, sizeof(buffer), 0);
				if (bytesRead <= 0)
				{
					iterUser->endCilent();
					std::cout << iter->fd << ": Client disconnected." << std::endl;
				}
				else
				{
					std::string receivedMessage(buffer, bytesRead);
					std::string oneMsg;
					std::stringstream ss(receivedMessage);
					std::string command;
					std::cout << iter->fd << " client Received message: " << receivedMessage << std::endl;

					while (std::getline(ss, oneMsg))
					{
						cm.exeCmd(oneMsg, iterUser);
					}
				}
				
				if (iterUser->getIsEnd())
				{
					std::cout << iter->fd << RED " CLIENT END" RESET << "\n\n";
					close(iter->fd);
					pollfds.erase(iter);
					users.erase(iterUser);

					std::cout << "remain clients :" << users.size() << " " << pollfds.size() << std::endl;
					break;
				}
			}
		}
	}
}