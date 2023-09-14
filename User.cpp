#include "User.hpp"

User::User() : socket_(-1) {}

User::User(int socket)
: socket_(socket), isPassed(false), hasNick(false), hasUser(false), isRegistered(false) {}

User::User(const User& user) {
	*this = user;
}

User& User::operator=(const User& user) {
	if (this != &user) {
		this->socket_ = user.socket_;
	}
	return *this;
}

User::~User() {}

int User::getSocket() {
	return this->socket_;
}

const std::string& User::getNickName() const {
	return this->nickName_;
}

const std::string& User::getUserName() const {
	return this->userName_;
}

const std::string& User::getHostName() const {
	return this->hostName_;
}

const std::string& User::getServerName() const {
	return this->serverName_;
}

const std::string& User::getRealName() const {
	return this->realName_;
}

bool User::getIsPassed() const {
	return this->isPassed;
}

bool User::getHasNick() const {
	return this->hasNick;
}

bool User::getHasUser() const {
	return this->hasUser;
}

bool User::getIsRegistered() const {
	return this->isRegistered;
}

void User::setIsPassed(bool flag) {
	this->isPassed = flag;
}
void User::setHasNick(bool flag) {
	this->hasNick = flag;
}
void User::setHasUser(bool flag) {
	this->hasUser = flag;
}
void User::setIsRegistered(bool flag) {
	this->isRegistered = flag;
}

void User::setNickName(const std::string& name) {
	this->nickName_ = name;
	this->hasNick = true;
	if (this->isPassed == true && this->hasUser == true)
		this->isRegistered == true;
}

void User::setUserName(const std::string& name) {
	this->userName_ = name;
	this->hasUser = true;
	if (this->isPassed == true && this->hasNick == true)
		this->isRegistered == true;
}

void User::setHostName(const std::string& name) {
	this->hostName_ = name;
}

void User::setServerName(const std::string& name) {
	this->serverName_ = name;
}

void User::setRealName(const std::string& name) {
	this->realName_ = name;
}