#pragma once

#include "Utils.hpp"

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
	CLIENT_MAX_BODY_SIZE_MUL,
	ALLOWED_METHODS,
	REDIRECT,
	RETURN,
	CGI,
	CGI_PATH,
	UPLOAD_STORE,
};

struct Token
{
	TokenType type;
	std::string value;
	int nums;
};

std::vector<Token> tokenize(std::string &input);
void SanitizeInput(std::string &input);
