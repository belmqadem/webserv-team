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
    std::cout << "Parse Listen " << value.value << std::endl;
}

void    Parser::parseServerNameDirective() {
    Token key = consume(SERVER_NAME);
    std::vector<std::string> serverNames;

    serverNames.push_back(consume(STRING).value);
    while (_tokens[_index].type == STRING)
        serverNames.push_back(consume(STRING).value);
    consume(SEMICOLON);

    std::cout << "parseServerNameDirective" << std::endl;
}

void    Parser::parseRootDirective() {
    Token   key = consume(ROOT);
    Token   value  = consume(STRING);
    consume(SEMICOLON);
    std::cout << "Parse Root " << value.value << std::endl;

}

void    Parser::parseErrorPageDirective() {
    Token key = consume(ERROR_PAGE);
    Token error_code = consume(NUMBER);
    Token file = consume(STRING);
    consume(SEMICOLON);
    std::cout << "Parsed error page: " << error_code.value << std::endl;
}

void Parser::parseLocationBlock() {
    Token key = consume(LOCATION);
    Token path = consume(STRING);
    consume(LBRACE);
    while (_tokens[_index].type != RBRACE) {
        parseDirecive();
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