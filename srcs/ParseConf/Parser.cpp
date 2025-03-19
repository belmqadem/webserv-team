#include "Parser.hpp"

bool isValidAddr(std::string addr)
{
	struct sockaddr_in s;
	return inet_pton(AF_INET, addr.c_str(), &(s.sin_addr)) == 1;
}

Token Parser::consume(TokenType expected)
{
	if (_tokens[_index].type == expected)
	{
		return _tokens[_index++];
	}
	throw std::runtime_error("Syntax Error: Unexpected token " + _tokens[_index].value);
}

std::pair<std::string, uint16_t> listenDirective(Token directive)
{

	uint16_t port;
	std::string host;
	if ((std::istringstream(directive.value) >> port).eof())
		return std::make_pair("0.0.0.0", port);
	host = directive.value;
	size_t colon;
	if ((colon = host.find(':')) != std::string::npos)
	{
		host = directive.value.substr(0, colon);
		std::istringstream isstream(directive.value.substr(colon + 1));
		isstream >> port;
		if (isstream.fail() || !isstream.eof())
		{
			throw std::runtime_error("Syntax Error: Unexpected token " + directive.value);
		}
	}
	if (!isValidAddr(host))
	{
		throw std::runtime_error("Syntax Error: Unexpected token " + host);
	}
	return std::make_pair(host, port);
}

void Parser::parseListenDirective()
{
	Token key = consume(LISTEN);
	Token value = consume(NUMBER);
	consume(SEMICOLON);
	if (_currentServer)
	{
		std::pair<std::string, uint16_t> listen = listenDirective(value);
		_currentServer->host = listen.first;
		_currentServer->port = listen.second;
	}
}

void Parser::parseServerNameDirective()
{
	Token key = consume(SERVER_NAME);
	std::vector<std::string> serverNames;

	serverNames.push_back(consume(STRING).value);
	while (_tokens[_index].type == STRING)
		serverNames.push_back(consume(STRING).value);
	consume(SEMICOLON);
	if (_currentServer)
		_currentServer->serverNames = serverNames;
}

void Parser::parseErrorPageDirective()
{
	Token key = consume(ERROR_PAGE);

	std::vector<int> error_codes;
	while (_tokens[_index].type == NUMBER)
	{
		Token err_code = consume(NUMBER);
		error_codes.push_back(std::atoi(err_code.value.c_str()));
	}
	Token file = consume(STRING);
	consume(SEMICOLON);
	if (_currentServer)
		for (size_t i = 0; i < error_codes.size(); i++)
		{
			_currentServer->errorPages[error_codes[i]] = file.value;
		}
}

// Parse a size string (like "10M", "1G", etc.)
size_t parseSize(const std::string &sizeStr)
{
	std::string numPart = sizeStr;
	size_t multiplier = 1;

	// Check for size suffix
	if (sizeStr.length() > 0)
	{
		char lastChar = sizeStr[sizeStr.length() - 1];
		if (lastChar == 'K' || lastChar == 'k')
		{
			numPart = sizeStr.substr(0, sizeStr.length() - 1);
			multiplier = SIZE_KB;
		}
		else if (lastChar == 'M' || lastChar == 'm')
		{
			numPart = sizeStr.substr(0, sizeStr.length() - 1);
			multiplier = SIZE_MB;
		}
		else if (lastChar == 'G' || lastChar == 'g')
		{
			numPart = sizeStr.substr(0, sizeStr.length() - 1);
			multiplier = SIZE_GB;
		}
	}

	// Convert to size_t and apply multiplier
	std::istringstream iss(numPart);
	size_t size;
	if (!(iss >> size))
	{
		throw std::runtime_error("Invalid size format: " + sizeStr);
	}

	return size * multiplier;
}

void Parser::parseRedirectDirective(Location &location)
{
	consume(REDIRECT);

	if (_tokens[_index].type == NUMBER)
	{
		Token code = consume(NUMBER);
		int statusCode = std::atoi(code.value.c_str());

		location.isRedirectPermanent = (statusCode == 301);

		location.redirectCode = statusCode;
	}
	else
	{
		location.isRedirectPermanent = false;
		location.redirectCode = 302;
	}

	Token url = consume(STRING);
	location.isRedirect = true;
	location.redirectUrl = url.value;

	consume(SEMICOLON);
}

void Parser::parseUploadStoreDirective(Location &location)
{
	consume(UPLOAD_STORE);
	Token path = consume(STRING);
	location.uploadStore = path.value;
	consume(SEMICOLON);
}

void Parser::parseClientMaxBodySizeDirective()
{
	consume(CLIENT_MAX_BODY_SIZE);
	Token size = consume(NUMBER);
	if (_currentServer)
	{
		_currentServer->clientMaxBodySize = parseSize(size.value);
	}
	consume(SEMICOLON);
}

void Parser::parseCgiDirective(Location &location)
{
	consume(CGI);

	if (_tokens[_index].type == STRING)
	{
		Token value = consume(STRING);
		if (value.value == "on")
		{
			location.useCgi = true;
		}
		else if (value.value == "off")
		{
			location.useCgi = false;
		}
		else
		{
			throw std::runtime_error("Syntax Error: Expected 'on' or 'off' after 'cgi', got '" + value.value + "'");
		}
	}
	else
	{
		throw std::runtime_error("Syntax Error: Expected 'on' or 'off' after 'cgi'");
	}
	consume(SEMICOLON);

	while (_tokens[_index].type == CGI_PATH ||
		   _tokens[_index].type == CGI_EXTENSION ||
		   _tokens[_index].type == CGI_PASS ||
		   _tokens[_index].type == CGI_WORKING_DIRECTORY)
	{

		if (_tokens[_index].type == CGI_PATH)
		{
			consume(CGI_PATH);
			Token path = consume(STRING);
			location.cgiPath = path.value;
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == CGI_EXTENSION)
		{
			consume(CGI_EXTENSION);
			Token extension = consume(STRING);
			Token handler = consume(STRING);
			location.cgiExtensions[extension.value] = handler.value;
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == CGI_PASS)
		{
			consume(CGI_PASS);
			Token interpreter = consume(STRING);
			location.cgiPath = interpreter.value; // Use existing field
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == CGI_WORKING_DIRECTORY)
		{
			consume(CGI_WORKING_DIRECTORY);
			Token workingDir = consume(STRING);
			location.cgiWorkingDirectory = workingDir.value; // Add this field to Location struct
			consume(SEMICOLON);
		}
	}
}

void Parser::parseLocationBlock()
{
	Token key = consume(LOCATION);
	Token path = consume(STRING);
	consume(LBRACE);
	Location location;
	location.location = path.value;

	while (_tokens[_index].type != RBRACE)
	{
		if (_tokens[_index].type == ROOT)
		{
			consume(ROOT);
			location.root = consume(STRING).value;
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == ERROR_PAGE)
		{
			Token key = consume(ERROR_PAGE);
			Token error_code = consume(NUMBER);
			Token file = consume(STRING);
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == AUTOINDEX)
		{
			consume(AUTOINDEX);
			Token value = consume(STRING);
			location.autoindex = (value.value == "on");
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == INDEX)
		{
			consume(INDEX);
			Token value = consume(STRING);
			location.index = value.value;
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == ALLOWED_METHODS)
		{
			consume(ALLOWED_METHODS);
			location.allowedMethods.clear();
			while (_tokens[_index].type == STRING)
			{
				location.allowedMethods.push_back(consume(STRING).value);
			}
			consume(SEMICOLON);
		}
		else if (_tokens[_index].type == REDIRECT)
		{
			parseRedirectDirective(location);
		}
		else if (_tokens[_index].type == CGI)
		{
			parseCgiDirective(location);
		}
		else if (_tokens[_index].type == UPLOAD_STORE)
		{
			parseUploadStoreDirective(location);
		}
		else
		{
			throw std::runtime_error("Syntax Error: Unknown directive in location block: " + _tokens[_index].value);
		}
	}

	consume(RBRACE);
	if (_currentServer)
		_currentServer->locations.push_back(location);
}

void Parser::parseDirecive()
{
	Token directiveToken = _tokens[_index];

	if (directiveToken.type == LISTEN)
	{
		parseListenDirective();
	}
	else if (directiveToken.type == SERVER_NAME)
	{
		parseServerNameDirective();
	}
	else if (directiveToken.type == ROOT)
	{
		// parseRootDirective();
		throw std::runtime_error("Configuration Error: 'root' directive is only allowed in location blocks");
	}
	else if (directiveToken.type == ERROR_PAGE)
	{
		parseErrorPageDirective();
	}
	else if (directiveToken.type == LOCATION)
	{
		parseLocationBlock();
	}
	else if (directiveToken.type == CLIENT_MAX_BODY_SIZE)
	{
		parseClientMaxBodySizeDirective();
	}
	else
	{
		throw std::runtime_error("Syntax Error : Unknown directive " + directiveToken.value);
	}
}

void Parser::parseServerBlock()
{
	consume(SERVER);
	consume(LBRACE);

	_servers.push_back(ServerConfig());
	_currentServer = &_servers.back();
	while (_tokens[_index].type != RBRACE)
	{
		parseDirecive();
	}
	consume(RBRACE);
	_currentServer = NULL;
}

void Parser::parseConfig()
{
	while (_tokens[_index].type != END)
	{
		parseServerBlock();
	}
}
