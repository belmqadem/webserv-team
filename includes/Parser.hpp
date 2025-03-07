#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include "Tokenize.hpp"

struct Location {
    std::string location;
    std::string root;
    std::map<int, std::string> errorPages;
    std::vector<std::string> allowedMethods;
    bool autoindex;
    std::string index;
    
    Location() : autoindex(false) {}
};

struct ServerConfig {
    uint16_t                    port;
    std::string                 host;
    std::vector<std::string>    serverNames;
    std::string                 root;
    std::map<int, std::string>  errorPages;
    std::vector<Location>       locations;
    
    ServerConfig() : port(80), host("0.0.0.0") {}
};

class Parser {
private:
    std::vector<Token> _tokens;
    size_t _index;
    std::vector<ServerConfig> _servers;
    ServerConfig* _currentServer;
    
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
    Parser(const std::vector<Token>& tokens) : _tokens(tokens), _index(0), _currentServer(NULL) {}
    
    const std::vector<ServerConfig>& getServers() const {
        return _servers;
    }
    
    friend class ConfigManager;
};

#endif
