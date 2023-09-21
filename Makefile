.DEFAULT_GOAL := all

NAME		= ircserv

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
RFLAG		= -r

OBJ_DIR		= obj
SRCS		= main.cpp Server.cpp User.cpp Channel.cpp CmdManager.cpp

OBJ			= $(SRCS:.cpp=.o)
OBJS		= $(addprefix $(OBJ_DIR)/, $(OBJ))

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

all: $(OBJ_DIR) $(NAME)

clean:
	$(RM) $(RFLAG) $(OBJ_DIR)

fclean: clean
	$(RM) $(RFLAG) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: all clean fclean re