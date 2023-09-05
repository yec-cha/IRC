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

const int SERVER_PORT = 6670; // IRC 서버 포트 번호
const int MAX_CONNECTIONS = 10; // 최대 연결 수

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
    // 사용자, 채널 및 기타 데이터를 관리하기 위한 데이터 구조를 정의해야 합니다.

	void handleClient(int clientSocket) {
        // 클라이언트와 연결되었음을 알림
        std::cout << "Client connected." << std::endl;

        // 클라이언트와 통신을 처리하는 코드를 작성
        char buffer[1024];
        while (true) {
            // 클라이언트로부터 데이터 수신
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                // 클라이언트 연결 종료 또는 오류 발생
                break;
            }

            // 수신한 데이터를 문자열로 변환
            std::string receivedMessage(buffer, bytesRead);

			// if (receivedMessage == "HI\n") {
            //     std::string response = "Hello, Client!\n";
            //     send(clientSocket, response.c_str(), response.length(), 0);
            // }
			std::cout << "--------------" << std::endl;
			std::cout << receivedMessage << std::endl;
			// send(clientSocket, receivedMessage.c_str(), receivedMessage.length(), 0);
			std::cout << "--------------" << std::endl;

            // "HI" 명령어를 처리
            if (receivedMessage == "HI\n") {
                std::string response = "Hello, Client!\n";
                send(clientSocket, response.c_str(), response.length(), 0);
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
    server.acceptConnections(); // 클라이언트 연결 수락 및 처리 시작
    return 0;
}
