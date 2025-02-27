#pragma once


#include <string>
#include <vector>
#include <sstream>
#include <iostream>

enum TokenType {SERVER , LISTEN, SERVER_NAME, NUMBER, STRING, LBRACE, RBRACE, SEMICOLON, END};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> tokenize(const std::string &input);