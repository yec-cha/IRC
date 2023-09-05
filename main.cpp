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

class User {
public:
    User(int socket, const std::string& nickname) : socket(socket), nickname(nickname) {}

    int getSocket() const {
        return socket;
    }

    std::string getNickname() const {
        return nickname;
    }

private:
    int socket;
    std::string nickname;
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

        std::cout << "IRC Server started on port " << SERVER_PORT << std::endl;
    }

    void acceptConnections() {
        while (true) {
            int clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket == -1) {
                std::cerr << "Error: Unable to accept client connection." << std::endl;
                continue;
            }

            // 클라이언트와 통신 시작
            handleClient(clientSocket);
        }
    }

private:
    int serverSocket;
    struct sockaddr_in serverAddress;
    std::vector<User> users;
    std::vector<Channel> channels;

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
        std::cout << "Client connected." << std::endl;

        // 클라이언트와 통신을 처리하는 코드를 작성
        char buffer[1024];
        while (true) {
            // poll 구조체 초기화
            struct pollfd pollfds[1];
            pollfds[0].fd = clientSocket;
            pollfds[0].events = POLLIN;

            int pollResult = poll(pollfds, 1, -1); // 무제한 대기

            if (pollResult == -1) {
                // 오류 처리
                std::cerr << "Error in poll." << std::endl;
                break;
            } else if (pollResult == 0) {
                // 타임아웃 처리
                continue;
            }

            if (pollfds[0].revents & POLLIN) {
                // 클라이언트로부터 데이터 수신
                ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    // 클라이언트 연결 종료 또는 오류 발생
                    break;
                }

                // 수신한 데이터를 문자열로 변환
                std::string receivedMessage(buffer, bytesRead);

                // 수신한 데이터 처리
                std::cout << "Received message: " << receivedMessage << std::endl;

                // 예시: "HI" 명령어를 처리
                if (receivedMessage == "HI\n") {
                    std::string response = "Hello, Client!\n";
                    send(clientSocket, response.c_str(), response.length(), 0);
                }
            }
        }

        // 클라이언트와의 통신이 끝나면 연결 종료
        close(clientSocket);

        // 연결이 종료되었음을 알림
        std::cout << "Client disconnected." << std::endl;
    }
};

int main() {
    IRCServer server;
    server.acceptConnections();
    return 0;
}
