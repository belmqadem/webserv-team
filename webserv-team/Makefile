# Makefile

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -I includes
SRC = src/ClientServer.cpp src/ConfigManager.cpp src/IOMultiplexer.cpp src/Logger.cpp src/Server.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = webserv

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re