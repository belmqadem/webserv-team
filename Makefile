GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror  -fsanitize=address -g #-std=c++98

UTILS = srcs/utils
REQUEST = srcs/request
CGI = srcs/CGI
SERVER = srcs/Server
# SRCS =	srcs/main.cpp \
# 		$(UTILS)/utils.cpp \
# 		$(REQUEST)/RequestParser.cpp \
# 		$(REQUEST)/ResponseBuilder.cpp
# SRCS =$(shell find srcs -name "*.cpp")
 SRCS =	srcs/main0.cpp \
		$(UTILS)/utils.cpp \
		$(REQUEST)/RequestParser.cpp \
		$(REQUEST)/ResponseBuilder.cpp \
		$(CGI)/CGIHandler.cpp \
		$(SERVER)/Logger.cpp \
		

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
