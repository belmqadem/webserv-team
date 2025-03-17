#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <limits>

enum TokenType
{
	SERVER,
	LISTEN,
	SERVER_NAME,
	ROOT,
	ERROR_PAGE,
	LOCATION,
	LBRACE,
	RBRACE,
	SEMICOLON,
	NUMBER,
	STRING,
	END,
	AUTOINDEX,
	INDEX,
	CLIENT_MAX_BODY_SIZE,
	ALLOWED_METHODS,
	REDIRECT,
	REDIRECT_PERMANENT,
	CGI,
	CGI_PATH,
	CGI_EXTENSION,
	CGI_PASS,
	CGI_WORKING_DIRECTORY,
	UPLOAD_STORE,
	MAX_UPLOAD_SIZE
};

struct Token
{
	TokenType type;
	std::string value;
};

std::vector<Token> tokenize(std::string &input);
void SanitizeInput(std::string &input);
bool find_del(char b);