#include "Parser.hpp"
#include "Readconfig.cpp"

Token   Parser::consume(TokenType expected)
{
    if (_tokens[_index].type == expected) {
        return _tokens[_index++];
    }
    throw std::runtime_error("Syntax Error: Unexpected token " + _tokens[_index].value);
}

void    Parser::parseListenDirective() {
    Token key = consume(LISTEN);
    Token value = consume(NUMBER);
    consume(SEMICOLON);
    if (_currentServer)
        _currentServer->port = std::atoi(value.value.c_str());
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
        _currentServer->root = value.type;

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
    }
    consume(RBRACE);
}

void    Parser::parseDirecive() {
    Token directiveToken = _tokens[_index];

    if (directiveToken.type == LISTEN)
        parseListenDirective();
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

int main(int ac, char **av) {

    (void)ac; (void)av;

    std::string config;
    config = readConfig(av[1]);
    std::vector<Token> tokens = tokenize(config);

    try {
        Parser parser(tokens);
        parser.parseConfig();

        std::vector<ServerConfig> servConfig = parser.getServers();
        for (size_t i = 0; i < servConfig.size(); i++) {
            std::cout << "server : " << i << " listens on port "<< servConfig[i].port << "\n";
        }
    } catch (std::runtime_error &e) {
        std::cout << "Catched in main : " << e.what() << "\n";
    } catch (std::logic_error &e) {
    }
    return 0;
}