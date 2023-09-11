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

const int SERVER_PORT = 6671;
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

class User {
private:
    int socket;
    std::string nickname;

public:
    User(int socket, const std::string& nickname) : socket(socket), nickname(nickname) {}

    int getSocket() const {
        return socket;
    }

    std::string getNickname() const {
        return nickname;
    }
};

class Channel {
public:
    Channel(const std::string& name) : name(name) {}

    void addUser(const User& user) {
        users.push_back(user);
    }

    void removeUser(int socket) {
        users.erase(std::remove_if(users.begin(), users.end(),
            [socket](const User& user) {
                return user.getSocket() == socket;
            }), users.end());
    }

    std::string getName() const {
        return name;
    }

    const std::vector<User>& getUsers() const {
        return users;
    }

private:
    std::string name;
    std::vector<User> users;
};



class IRCServer {
private:
    int serverSocket;
    struct sockaddr_in serverAddress, clientAddr;
    std::vector<User> users;
    std::vector<Channel> channels;
    struct pollfd pollfds[MAX_CONNECTIONS];
    char buffer[1024];

    // 사용자 추가
    void addUser(int clientSocket, const std::string& nickname) {
        User user(clientSocket, nickname);
        users.push_back(user);
    }

    // 사용자 제거
    void removeUser(int clientSocket) {
        users.erase(std::remove_if(users.begin(), users.end(),
            [clientSocket](const User& user) {
                return user.getSocket() == clientSocket;
            }), users.end());
        
        // 채널에서도 해당 사용자 제거
        for (Channel& channel : channels) {
            channel.removeUser(clientSocket);
        }
    }

    // 채널 추가
    void addChannel(const std::string& channelName) {
        Channel channel(channelName);
        channels.push_back(channel);
    }

    // 채널 제거
    void removeChannel(const std::string& channelName) {
        channels.erase(std::remove_if(channels.begin(), channels.end(),
            [channelName](const Channel& channel) {
                return channel.getName() == channelName;
            }), channels.end());
    }

    // 사용자가 채널에 가입
    void joinChannel(int clientSocket, const std::string& channelName) {
        for (Channel& channel : channels) {
            if (channel.getName() == channelName) {
                channel.addUser(getUserBySocket(clientSocket));
                break;
            }
        }
    }

    // 사용자가 채널에서 나가기
    void leaveChannel(int clientSocket, const std::string& channelName) {
        for (Channel& channel : channels) {
            if (channel.getName() == channelName) {
                channel.removeUser(clientSocket);
                break;
            }
        }
    }

    // 소켓을 통해 사용자 찾기
    User getUserBySocket(int socket) {
        for (const User& user : users) {
            if (user.getSocket() == socket) {
                return user;
            }
        }
        throw std::runtime_error("User not found");
    }


    void handleClient(int clientSocket) {
        // 클라이언트와 연결되었음을 알림

        for (size_t i = 1; i < MAX_CONNECTIONS; i++)
        {
            if (pollfds[i].fd == -1)
            {
                pollfds[i].fd = clientSocket;
                pollfds[i].events = POLLIN;
                std::cout << i << " : Client connected." << std::endl;
                break ; 
            }
        }
    }
public:
    IRCServer() {
        // 서버 초기화
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "Error: Unable to create server socket." << std::endl;
            exit(1);
        }

        // 서버 주소 설정
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(SERVER_PORT);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        // 서버 바인딩
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            std::cerr << "Error: Binding failed." << std::endl;
            exit(1);
        }

        // 서버 리스닝
        if (listen(serverSocket, MAX_CONNECTIONS) == -1) {
            std::cerr << "Error: Listening failed." << std::endl;
            exit(1);
        }

        pollfds[0].fd = serverSocket;
        pollfds[0].events = POLLIN;

        for (size_t i = 1; i < MAX_CONNECTIONS; i++)
        {
            pollfds[i].fd = -1;
        }
        
        std::cout << "IRC Server started on port " << SERVER_PORT << std::endl;
    }

    void acceptConnections() {
        int clientLen = sizeof(clientAddr);
        int clientSocket;
        int pollResult;

        while (true) {
            pollResult = poll(pollfds, MAX_CONNECTIONS, -1);
            if (pollResult == -1) {
                // 오류 처리
                std::cerr << "Error in poll." << std::endl;
                break;
            } else if (pollResult == 0) {
                // 타임아웃 처리
                continue;
            }
            
            if (pollfds[0].revents & POLLIN)
            {
                clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t *)&clientLen);
                if (clientSocket == -1) {
                    std::cerr << "Error: Unable to accept client connection." << std::endl;
                    continue;
                }
                handleClient(clientSocket);
            }

            for (size_t i = 1; i < MAX_CONNECTIONS; i++)
            {
                if (pollfds[i].fd > 0 && pollfds[i].revents & POLLIN)
                {
                // 클라이언트로부터 데이터 수신
                ssize_t bytesRead = recv(pollfds[i].fd, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    close(pollfds[i].fd);
                    pollfds[i].fd = -1;
                    std::cout << i << ": Client disconnected." << std::endl;
                    break;
                }


                // 수신한 데이터를 문자열로 변환
                std::string receivedMessage(buffer, bytesRead);

                // 수신한 데이터 처리
                std::cout << i << " client Received message: " << receivedMessage << std::endl;
                
                //std::size_t found = receivedMessage.find("CAP LS");
                //if (found!=std::string::npos) {
                //    std::cout << "We find CAP LS" << std::endl;
                    
                //    std::string response = "CAP LS\n";
                //    send(clientSocket, response.c_str(), response.length(), 0);
                //}
				if (receivedMessage == "JOIN :\n")
				{
					std::cout << "join\n";
					std::string response = "461";
                    send(clientSocket, response.c_str(), response.length(), 0);
				}
                // 예시: "HI" 명령어를 처리
                if (receivedMessage == "HI\n") {
                    std::string response = "Hello, Client!\n";
                    send(clientSocket, response.c_str(), response.length(), 0);
                }
                }
                if (pollfds[i].revents & (POLLHUP | POLLERR))
                {
                    close(pollfds[i].fd) ;
                    std::cout << pollfds[i].fd << ": Client disconnected." << std::endl;
                }
            }
            
            
        }
    }
};

int main() {
    IRCServer server;
    server.acceptConnections();
    return 0;
}
