GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror  -fsanitize=address -g3 -std=c++98
SRCS = $(shell find srcs -name "*.cpp")
OBJS = $(SRCS:.cpp=.o)
INC=includes

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(INC)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(NAME) 
	@echo "$(GREEN)$(NAME) compiled.$(RESET)"

clean :
	@echo -n "$(RED)"
	rm -f $(OBJS)
	@echo -n "$(RESET)"

fclean :
	@echo -n "$(RED)"
	rm -f $(OBJS) $(NAME)
	@echo -n "$(RESET)"

re : fclean all

.PHONY : all clean fclean re
.SECONDARY : $(OBJS)
