#include "Server.hpp"

int checkArguments(int argc, char** argv) {
    if (argc != 3)
        return -1;
    std::string port(argv[1]);
    if (port.size() != 4)
        return -1;
    for (std::string::iterator iter = port.begin(); iter < port.end(); ++iter) {
        if (std::isdigit(*iter) == false)
            return -1;
    }
    int result = std::atoi(argv[1]);
    if (result == 0)
        return -1;
	std::string pass(argv[2]);
	if (pass.size() <= 0)
		return -1;
    return result;
}

int main(int argc, char** argv) {
	int port = checkArguments(argc, argv);
    if (port == -1)
        return 1;
    
	Server server(port, argv[2]);

	struct sigaction sig;
	sig.sa_handler = server.signal_handler;
	sigaction(SIGINT, &sig, 0);
	sigaction(SIGQUIT, &sig, 0);

	server.acceptConnections();
	return 0;
}
