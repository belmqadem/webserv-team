#include "Parser.hpp"
#include "ConfigManager.hpp"
#include <sstream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


bool    isValidAddr(std::string addr) {
    struct sockaddr_in s;
    return inet_pton(AF_INET, addr.c_str(), &(s.sin_addr)) == 1;
}

Token   Parser::consume(TokenType expected)
{
    if (_tokens[_index].type == expected) {
        return _tokens[_index++];
    }
    throw std::runtime_error("Syntax Error: Unexpected token " + _tokens[_index].value);
}

std::pair<std::string, uint16_t> listenDirective(Token directive) {
    
    uint16_t    port;
    std::string host;
    if ((std::istringstream(directive.value) >> port).eof())
        return std::make_pair("0.0.0.0", port);
    host = directive.value;
    size_t colon;
    if ((colon = host.find(':')) != std::string::npos) {
        host = directive.value.substr(0, colon);
        std::istringstream isstream(directive.value.substr(colon + 1));
        isstream >> port;
        if (isstream.fail() || !isstream.eof()) {
            throw std::runtime_error("Syntax Error: Unexpected token " + directive.value);
        }
    }
    if (!isValidAddr(host)) {
        throw std::runtime_error("Syntax Error: Unexpected token " + host);
    }
    return std::make_pair(host, port);
}

void    Parser::parseListenDirective() {
    Token key = consume(LISTEN);
    Token value = consume(NUMBER);
    consume(SEMICOLON);
    if (_currentServer) {
        std::pair<std::string, uint16_t> listen = listenDirective(value);
        _currentServer->host = listen.first;
        _currentServer->port = listen.second;
    }
}

void    Parser::parseServerNameDirective() {
    Token key = consume(SERVER_NAME);
    std::vector<std::string> serverNames;

    serverNames.push_back(consume(STRING).value);
    while (_tokens[_index].type == STRING)
        serverNames.push_back(consume(STRING).value);
    consume(SEMICOLON);
    if (_currentServer)
        _currentServer->serverNames = serverNames;
}

void    Parser::parseRootDirective() {
    Token   key = consume(ROOT);
    Token   value  = consume(STRING);
    consume(SEMICOLON);
    if (_currentServer)
        _currentServer->root = value.value; // Fixed: Using value.value instead of value.type

}

void    Parser::parseErrorPageDirective() {
    Token key = consume(ERROR_PAGE);
    Token error_code = consume(NUMBER);
    Token file = consume(STRING);
    consume(SEMICOLON);
    if (_currentServer)
        _currentServer->errorPages[std::atoi(error_code.value.c_str())] = file.value;
}

void Parser::parseLocationBlock() {
    Token key = consume(LOCATION);
    Token path = consume(STRING);
    consume(LBRACE);
    Location location;
    location.location = path.value;
    while (_tokens[_index].type != RBRACE) {
        if (_tokens[_index].type == ROOT) {
            consume(ROOT);
            location.root = consume(STRING).value;
            consume(SEMICOLON);
        }
        else if (_tokens[_index].type == ERROR_PAGE) {
            Token key = consume(ERROR_PAGE);
            Token error_code = consume(NUMBER);
            Token file = consume(STRING);
            consume(SEMICOLON);
            location.errorPages[std::atoi(error_code.value.c_str())] = file.value;
        }
    }
    consume(RBRACE);
    if (_currentServer)
        _currentServer->locations.push_back(location);
}

void    Parser::parseDirecive() {
    Token directiveToken = _tokens[_index];

    if (directiveToken.type == LISTEN)
    {
        parseListenDirective();
    }
    else if (directiveToken.type == SERVER_NAME)
        parseServerNameDirective();
    else if (directiveToken.type == ROOT)
        parseRootDirective();
    else if (directiveToken.type == ERROR_PAGE)
        parseErrorPageDirective();
    else if (directiveToken.type == LOCATION)
        parseLocationBlock();
    else
        throw std::runtime_error("Syntax Error : Uknown directive " + directiveToken.value);
}

void    Parser::parseServerBlock() {
    consume(SERVER);
    consume(LBRACE);

    _servers.push_back(ServerConfig());
    _currentServer = &_servers.back();
    while (_tokens[_index].type != RBRACE) {
        parseDirecive();
    }
    consume(RBRACE);
    _currentServer = NULL;
}

void    Parser::parseConfig() {
    while (_tokens[_index].type != END)
    {
        parseServerBlock();
    }
}

// int main(int ac, char **av) {
//     if (ac != 2) {
//         std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
//         return 1;
//     }

//     ConfigManager* configManager = ConfigManager::getInstance();
    
//     if (!configManager->loadConfig(av[1])) {
//         std::cerr << "Failed to load configuration." << std::endl;
//         return 1;
//     }
    
//     const std::vector<ServerConfig>& servers = configManager->getServers();
    
//     // Display information about loaded servers
//     for (size_t i = 0; i < servers.size(); i++) {
//         std::cout << "Server " << i << " listening on port " << servers[i].port << std::endl;
        
//         if (!servers[i].serverNames.empty()) {
//             std::cout << "  Server names: ";
//             for (size_t j = 0; j < servers[i].serverNames.size(); j++) {
//                 std::cout << servers[i].serverNames[j] << " ";
//             }
//             std::cout << std::endl;
//         }
        
//         std::cout << "  Root: " << servers[i].root << std::endl;
        
//         if (!servers[i].locations.empty()) {
//             std::cout << "  Locations: " << servers[i].locations.size() << std::endl;
//         }
//     }
    
//     // Clean up the singleton instance
//     ConfigManager::destroyInstance();
    
//     return 0;
// }