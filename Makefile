GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

NAME = webserv
CONFIG_TESTER = config_tester
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -g3
SRCS = $(shell find srcs -name "*.cpp" -not -name "test_parser.cpp")
OBJS = $(SRCS:.cpp=.o)

# Sources for the config tester
CONFIG_SRCS = srcs/ParseConf/ConfigManager.cpp \
			  srcs/ParseConf/Parser.cpp \
			  srcs/ParseConf/Tokenize.cpp \
			  srcs/test_parser.cpp
CONFIG_OBJS = $(CONFIG_SRCS:.cpp=.o)

INC=includes

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(INC)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(NAME) 
	@echo "$(GREEN)$(NAME) compiled.$(RESET)"

# New target for the config tester
CONF: $(CONFIG_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(CONFIG_TESTER)
	@echo "$(GREEN)$(CONFIG_TESTER) compiled.$(RESET)"

clean :
	@echo -n "$(RED)"
	rm -f $(OBJS) $(CONFIG_OBJS)
	@echo -n "$(RESET)"

fclean :
	@echo -n "$(RED)"
	rm -f $(OBJS) $(CONFIG_OBJS) $(NAME) $(CONFIG_TESTER)
	@echo -n "$(RESET)"

re : fclean all

.PHONY : all clean fclean re CONF
.SECONDARY : $(OBJS) $(CONFIG_OBJS)