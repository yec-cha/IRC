#include "Server.hpp"

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

int checkArguments(int argc, char** argv) {
    if (argc != 3)
        return -1;
    std::string port(argv[1]);
    if (port.size() != 4)
        return -1;
    // std::string::iterator iter = port.begin();
    for (std::string::iterator iter = port.begin(); iter < port.end(); ++iter) {
        if (std::isdigit(*iter) == false)
            return -1;
    }
    int result = std::atoi(argv[1]);
    if (result == 0)
        return -1; // need to add
    return result;
}

int main(int argc, char** argv) {
	int port = checkArguments(argc, argv);
    if (port == -1)
        return 1;
    
    IRCServer server(port);
	server.acceptConnections();
	return 0;
}
