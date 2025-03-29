NAME = logbot

CC					=	g++
CXXFLAGS			=	-Wall -Wextra -Werror -std=c++17
INCLUDES			=	-I.
OBJ_DIR				=	./obj
SRC_DIR				=	.

SRCS				=	IRCBot.cpp \
						LogBot.cpp \
						main.cpp

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

.PHONY: all re clean fclean

GREEN = \033[0;32m
RESET = \033[0m
BLUE = \033[34m
