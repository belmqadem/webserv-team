#pragma once

#include "Tokenize.hpp"

class Parser
{
private:
    std::vector<Token> _tokens;
    size_t _index;

public:
    Parser(std::vector<Token> t) : _tokens(t), _index(0) {}
    Token consume(TokenType expected);

    void parseDirecive();
    void parseListenDirective();
    void parseServerNameDirective();
    void parseRootDirective();
    void parseErrorPageDirective();
    void parseLocationBlock();
    void parseServerBlock();
    void parseConfig();
};
