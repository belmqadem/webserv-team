#include "Tokenize.hpp"
#include <algorithm>

bool find_del(char b) {
    std::string delim = ";{}"; 
    return (delim.find(b) != std::string::npos);
}

void SanitizeInput(std::string &input) {
    for (size_t i = 0; i < input.length(); i++) {
        if (find_del(input[i]))
        {
            input.insert(i, " ");
            input.insert(i + 2, " ");
            i+= 2;
        }
    }
}

std::vector<Token> tokenize(std::string &input) {
    std::vector<Token> tokens;
    SanitizeInput(input);
    std::cout << "SanitizedInput : " << input << std::endl;
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
        else if (word == "root") {
            token.type = ROOT;
            token.value = word;
            tokens.push_back(token);
        }
        else if (word == "error_page") {
            token.type = ERROR_PAGE;
            token.value = word;
            tokens.push_back(token);
        }
        else if (word == "location") {
            token.type = LOCATION;
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
