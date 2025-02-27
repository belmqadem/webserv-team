#include "Parser.hpp"
#include "Readconfig.cpp"

Token   Parser::consume(TokenType expected)
{
    if (_tokens[_index].type == expected) {
        return _tokens[_index++];
    }
    throw std::runtime_error("Syntax Error: Unexpected token " + _tokens[_index].value);
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
    
}

void    Parser::parseServerBlock() {
    consume(SERVER);
    consume(LBRACE);
    while (_tokens[_index].type != RBRACE) {
        parseDirecive();
    }
    consume(RBRACE);
    std::cout << "parseServerBlock Done\n";
}

void    Parser::parseConfig() {
    while (_tokens[_index].type != END)
    {
        parseServerBlock();
    }
    std::cout << "Parsing is Done!\n";
}

int main(int ac, char **av) {

    (void)ac; (void)av;

    std::string config;
    config = readConfig(av[1]);
    std::vector<Token> tokens = tokenize(config);

    try {
        Parser parser(tokens);
        parser.parseConfig();
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}