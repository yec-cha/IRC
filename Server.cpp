#include "Server.hpp"

std::vector<struct pollfd> Server::pollfds;

Server::Server() : cm(users, channels, pass) {}

Server::Server(int port, const std::string _pass) : cm(users, channels, pass), pass(_pass), port_(port)
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
	if (listen(this->serverSocket, SOMAXCONN) == -1)
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

Server::Server(const Server &server) : cm(users, channels, pass)
{
	*this = server;
}

Server &Server::operator=(const Server &server)
{
	if (this != &server)
	{
		this->serverSocket = server.serverSocket;
		this->serverAddress = server.serverAddress;
		this->clientAddr = server.clientAddr;
		this->users = server.users;
		this->channels = server.channels;
		this->port_ = server.port_;
	}
	return *this;
}

Server::~Server() {}

void Server::handleClient(int clientSocket)
{
	struct pollfd tmp;
	User newuser(clientSocket);

	tmp.fd = clientSocket;
	tmp.events = POLLIN;
	pollfds.push_back(tmp);
	users.push_back(newuser);
	std::cout << clientSocket << " : Client connected." << std::endl;
}

void Server::acceptConnections()
{
	int clientLen = sizeof(clientAddr);
	int clientSocket;
	int pollResult;
	std::deque<User>::iterator iterUser;
	std::vector<struct pollfd>::iterator iter;

	while (true)
	{
		pollResult = poll(&pollfds[0], pollfds.size(), 0);
		if (pollResult == -1)
		{
			std::cerr << "Error in poll." << std::endl;
			break;
		}
		else if (pollResult == 0)
			continue;
		if (pollfds[0].revents & POLLIN)
		{
			clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr *>(&clientAddr), reinterpret_cast<socklen_t *>(&clientLen));
			if (clientSocket == -1)
			{
				std::cerr << "Error: Unable to accept client connection." << std::endl;
				continue;
			}
			handleClient(clientSocket);
			std::cout << BLUE << users.size() << " " << users[0].getIsPassed() << RESET << std::endl;
		}

		for (iter = pollfds.begin() + 1, iterUser = users.begin(); (iter != pollfds.end()) && (iterUser != users.end()); iter++, iterUser++)
		{
			std::memset(buffer, 0, 532);
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
					std::cout << iter->fd << " client Received message: " << receivedMessage << std::endl;
					iterUser->setBuffer(iterUser->getBuffer() + receivedMessage);
					if (iterUser->getBuffer().find('\n') != std::string::npos)
					{
						std::string oneMsg;
						std::stringstream ss(iterUser->getBuffer());
						while (std::getline(ss, oneMsg))
							cm.exeCmd(oneMsg, iterUser);
						iterUser->setBuffer("");
					}
				}
				if (iterUser->getIsEnd())
				{
					std::cout << iter->fd << RED " CLIENT END" RESET << "\n\n";
					close(iter->fd);
					for (std::deque<Channel>::iterator iterCH = channels.begin(); iterCH != channels.end(); iterCH++)
						iterCH->deleteUser(iterUser->getSocket());
					pollfds.erase(iter);
					users.erase(iterUser);
					std::cout << "remain clients :" << users.size() << " " << pollfds.size() << std::endl;
					break;
				}
			}
		}
	}
}

void Server::endServer(const std::string &msg)
{
	for (size_t i = 1; i < pollfds.size(); i++)
	{
		send(pollfds[i].fd, msg.c_str(), msg.size(), MSG_DONTWAIT);
		close(pollfds[i].fd);
	}
	pollfds.clear();
}

void Server::signal_handler(int sig)
{
	if (sig == SIGQUIT || sig == SIGINT)
	{
		std::string quitMsg = ":ft_IRC QUIT :server end now";
		endServer(quitMsg);
		std::cout << BOLDMAGENTA << "\n\nSERVER END\n"<< RESET << std::endl;
		exit(0);
	}
}