#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <map>

class Channel {
private:
	std::string name_;
	std::string topic_;
	std::string pass_;
	std::map<int, User&> list_;
	int operator_;

public:
	Channel(const std::string &name, User& user) : name_(name) {
		addUser(user);
		setOperator(user.getSocket());
	}

	void addUser(User& user) {
		this->list_.insert(std::pair<int, User&>(user.getSocket(), user));
	}

	void delUser(User& user) {
		this->list_.erase(user.getSocket());
	}

	void setTopic(const std::string& topic) {
		this->topic_ = topic;
	}

	void setPass(const std::string& pass) {
		this->topic_ = pass;
	}

	void setOperator(int socket) {
		this->operator_ = socket;
	}

	const std::string& getName() const {
		return this->name_;
	}

	const std::string& getTopic() const {
		return this->topic_;
	}
	
	const std::string& getPass() const {
		return this->pass_;
	}

	int getOperator() const {
		return this->operator_;
	}
};

#endif