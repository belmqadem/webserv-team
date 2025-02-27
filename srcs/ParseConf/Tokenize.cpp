#include "Tokenize.hpp"

std::vector<Token> tokenize(const std::string &input) {
    std::vector<Token> tokens;
    std::istringstream stream(input);
    std::string word;

    while (stream >> word) {
        Token token;
        
        if (word == "server") {
            token.type = SERVER;
            token.value = word;
            tokens.push_back(token);
        }
        else if (word == "listen") {
            token.type = LISTEN;
            token.value = word;
            tokens.push_back(token);
        }
        else if (word == "server_name") {
            token.type = SERVER_NAME;
            token.value = word;
            tokens.push_back(token);
        }
        else if (word == "{") {
            token.type = LBRACE;
            token.value = word;
            tokens.push_back(token);
        }
        else if (word == "}") {
            token.type = RBRACE;
            token.value = word;
            tokens.push_back(token);
        }
        else if (word == ";") {
            token.type = SEMICOLON;
            token.value = word;
            tokens.push_back(token);
        }
        else if (isdigit(word[0])) {
            token.type = NUMBER;
            token.value = word;
            tokens.push_back(token);
        }
        else {
            token.type = STRING;
            token.value = word;
            tokens.push_back(token);
        }
    }
    Token endToken;
    endToken.type = END;
    endToken.value = "";
    tokens.push_back(endToken);
    
    return tokens;
}
