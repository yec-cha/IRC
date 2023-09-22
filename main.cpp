#include "Server.hpp"

std::string& ltrim(std::string& str, const char* t = " \t\n\r\f\v") {
	str.erase(0, str.find_first_not_of(t));
	return str;
}

std::string& rtrim(std::string& str, const char* t = " \t\n\r\f\v") {
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}

std::string& trim(std::string& str, const char* t = " \t\n\r\f\v") {
	return ltrim(rtrim(str, t), t);
}

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
	{
        return -1;
	}
	std::string pass(argv[2]);
    trim(pass);
	if (pass.size() <= 0)
	{
		return -1;
	}
    std::cout << "PASSWORD: [" << pass << "]" << std::endl;
    return result;
}

int main(int argc, char** argv) {
    int port = checkArguments(argc, argv);
    if (port == -1)
	{
        return 1;
	}
	
	std::string pass(argv[2]);
	Server server(port, trim(pass));

	struct sigaction sig;
	sig.sa_handler = server.signal_handler;
	sigaction(SIGINT, &sig, 0);
	sigaction(SIGQUIT, &sig, 0);

	server.acceptConnections();
	return 0;
}
