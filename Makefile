NAME = ircserv

CC					=	c++
CXXFLAGS			=	-std=c++17 -Wall -Wextra -Werror -g
INCLUDES			=	-Iinclude
OBJ_DIR				=	./obj
SRC_DIR				=	.

SRCS				=	src/main.cpp \
                        src/channel/Channel.cpp \
                        src/commands/cap.cpp \
                        src/commands/invite.cpp \
                        src/commands/join.cpp \
                        src/commands/kick.cpp \
                        src/commands/mode.cpp \
                        src/commands/motd.cpp \
                        src/commands/nick.cpp \
                        src/commands/part.cpp \
                        src/commands/pass.cpp \
                        src/commands/ping.cpp \
                        src/commands/pong.cpp \
                        src/commands/privmsg.cpp \
                        src/commands/quit.cpp \
                        src/commands/silentIgnore.cpp \
                        src/commands/topic.cpp \
                        src/commands/user.cpp \
                        src/commands/notice.cpp \
                        src/error/Error.cpp \
                        src/proxy/CommandRunner.cpp \
                        src/proxy/MessageParser.cpp \
                        src/server/ConnectionManager.cpp \
                        src/server/Server.cpp \
                        src/utils/IRCValidator.cpp \
                        src/server/Client.cpp \
                        src/server/ClientIndex.cpp \
                        src/server/SocketManager.cpp \
                        src/server/PongManager.cpp \
                        src/server/ChannelManager.cpp \
                        src/server/EventLoopEpoll.cpp \
                        src/utils/EventLoopFactory.cpp

OBJECTS				=	$(addprefix $(OBJ_DIR)/, $(SRCS:%.cpp=%.o))

vpath %.cpp $(SRC_DIR)

################################################################################
# REQUIRED
################################################################################

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CXXFLAGS) $(OBJECTS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

re: fclean all


################################################################################
# CLEAN
################################################################################

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(NAME)

################################################################################
# PHONY
################################################################################

.PHONY: all re clean fclean run

GREEN = \033[0;32m
RESET = \033[0m
BLUE = \033[34m