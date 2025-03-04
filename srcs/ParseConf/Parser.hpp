#pragma once

#include "Tokenize.hpp"
#include <map>
#include <stdlib.h>

struct Location
{
    std::string location;
    std::string root;
    // further params
};

struct ServerConfig
{
    int port;
    std::vector<std::string> serverNames;
    std::string root;
    std::map<int, std::string> errorPages;
    std::vector<Location> locations;
};

class Parser
{
private:
    std::vector<Token> _tokens;
    size_t _index;
    std::vector<ServerConfig> _servers;
    ServerConfig *_currentServer;

public:
    Parser(std::vector<Token> t) : _tokens(t), _index(0) {}
    Token consume(TokenType expected);
    const std::vector<ServerConfig> & getServers() const { return _servers; }

    void parseDirecive();
    void parseListenDirective();
    void parseServerNameDirective();
    void parseRootDirective();
    void parseErrorPageDirective();
    void parseLocationBlock();
    void parseServerBlock();
    void parseConfig();
};
