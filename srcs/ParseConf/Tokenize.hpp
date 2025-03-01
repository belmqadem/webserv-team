#pragma once


#include <string>
#include <vector>
#include <sstream>
#include <iostream>

enum TokenType {SERVER , LISTEN, SERVER_NAME, ROOT, ERROR_PAGE, LOCATION, NUMBER, STRING, LBRACE, RBRACE, SEMICOLON, END};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> tokenize(std::string &input);
void SanitizeInput(std::string &input);
bool find_del(char b);