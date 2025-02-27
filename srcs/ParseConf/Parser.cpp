#include "Parser.hpp"


Token   Parser::consume(TokenType expected)
{
    if (_tokens[_index].type == expected) {
        return _tokens[_index++];
    }
    throw std::runtime_error("Syntax Error: Unexpected token " + _tokens[_index].value);
}

void    Parser::parseDirecive() {
    Token key;
    Token value;

    key = consume(LISTEN);
    value = consume(NUMBER);
    consume(SEMICOLON);
    std::cout << "Parsed Directive: " << key.value << " " << value.value << std::endl;
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

    std::string config = "server { listen 8080 ; }";
    std::vector<Token> tokens = tokenize(config);

    try {
        Parser parser(tokens);
        parser.parseConfig();
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}