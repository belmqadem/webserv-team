#pragma once

#include "Tokenize.hpp"
#include "webserv.hpp"
#include "ConfigManager.hpp"
#include "Utils.hpp"

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
	void parseErrorPageDirective();
	void parseLocationBlock();

	void parseRedirectDirective(Location &location);
	void parseCgiDirective(Location &location);
	void parseUploadStoreDirective(Location &location);
	void parseClientMaxBodySizeDirective();
	void parseReturnDirective(Location &location);

	std::pair<std::string, uint16_t> listenDirective(Token directive);
	size_t parseSize(const std::string &sizeStr, char unit);
	bool isValidAddr(std::string addr);

public:
	Parser(const std::vector<Token> &tokens);

	const std::vector<ServerConfig> &getServers() const;

	friend class ConfigManager;
};
