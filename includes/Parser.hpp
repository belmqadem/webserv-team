#pragma once

#include "Tokenize.hpp"
#include "webserv.hpp"

struct Location
{
	/* TO ADD
		redirection ==> ie: (rewrite ^/old-page$ /new-page permanent)
		execute cgi ==> (cgi_pass /usr/bin/php-cgi)
		define where uploaded files should be saved ==> (upload_store /var/www/uploads)
	*/
	std::string location;
	std::string root;
	std::map<int, std::string> errorPages;
	std::vector<std::string> allowedMethods;
	bool autoindex;
	std::string index;

	Location() : autoindex(false) {}
};

struct ServerConfig
{
	/* TO ADD
		client_max_body_size
	*/
	uint16_t port;
	std::string host;
	std::vector<std::string> serverNames;
	std::string root;
	std::map<int, std::string> errorPages;
	std::vector<Location> locations;

	ServerConfig() : port(80), host("0.0.0.0") {}
};

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

public:
	Parser(const std::vector<Token> &tokens) : _tokens(tokens), _index(0), _currentServer(NULL) {}

	const std::vector<ServerConfig> &getServers() const
	{
		return _servers;
	}

	friend class ConfigManager;
};
