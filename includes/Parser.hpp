#pragma once

#include "Tokenize.hpp"
#include "webserv.hpp"
#include "ConfigManager.hpp"

struct ServerConfig;
struct Location;

class Parser
{
private:
	std::vector<Token> _tokens;
	size_t _index;
	std::vector<ServerConfig> _servers;
	ServerConfig *_currentServer;

	Token consume(TokenType expected);

	void parseConfig();
	void parseServerBlock();
	void parseDirecive();
	void parseListenDirective();
	void parseServerNameDirective();
	void parseRootDirective();
	void parseErrorPageDirective();
	void parseLocationBlock();

	void parseRedirectDirective(Location &location);
	void parseCgiDirective(Location &location);
	void parseUploadStoreDirective(Location &location);
	void parseClientMaxBodySizeDirective();

public:
	Parser(const std::vector<Token> &tokens) : _tokens(tokens), _index(0), _currentServer(NULL) {}

	const std::vector<ServerConfig> &getServers() const
	{
		return _servers;
	}

	friend class ConfigManager;
};
