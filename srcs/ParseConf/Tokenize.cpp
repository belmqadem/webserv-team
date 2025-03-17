#include "Tokenize.hpp"
#include <algorithm>

bool find_del(char b)
{
	std::string delim = ";{}";
	return (delim.find(b) != std::string::npos);
}

void SanitizeInput(std::string &input)
{
	for (size_t i = 0; i < input.length(); i++)
	{
		if (find_del(input[i]))
		{
			input.insert(i, " ");
			input.insert(i + 2, " ");
			i += 2;
		}
	}
}

std::vector<Token> tokenize(std::string &input)
{
	std::vector<Token> tokens;
	SanitizeInput(input);
	// std::cout << "SanitizedInput : " << input << std::endl;
	std::istringstream stream(input);
	std::string word;

	while (stream >> word)
	{
		if (word == "#")
		{
			stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}
		Token token;
		if (word == "server")
		{
			token.type = SERVER;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "listen")
		{
			token.type = LISTEN;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "root")
		{
			token.type = ROOT;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "error_page")
		{
			token.type = ERROR_PAGE;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "location")
		{
			token.type = LOCATION;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "autoindex")
		{
			token.type = AUTOINDEX;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "index")
		{
			token.type = INDEX;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "client_max_body_size")
		{
			token.type = CLIENT_MAX_BODY_SIZE;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "allowed_methods")
		{
			token.type = ALLOWED_METHODS;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "redirect")
		{
			token.type = REDIRECT;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "redirect_permanent")
		{
			token.type = REDIRECT_PERMANENT;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "cgi")
		{
			token.type = CGI;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "cgi_path")
		{
			token.type = CGI_PATH;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "cgi_extension")
		{
			token.type = CGI_EXTENSION;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "cgi_pass")
		{
			token.type = CGI_PASS;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "cgi_working_directory")
		{
			token.type = CGI_WORKING_DIRECTORY;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "upload_store")
		{
			token.type = UPLOAD_STORE;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "max_upload_size")
		{
			token.type = MAX_UPLOAD_SIZE;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "server_name")
		{
			token.type = SERVER_NAME;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "{")
		{
			token.type = LBRACE;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == "}")
		{
			token.type = RBRACE;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word == ";")
		{
			token.type = SEMICOLON;
			token.value = word;
			tokens.push_back(token);
		}
		else if (isdigit(word[0]))
		{
			token.type = NUMBER;
			token.value = word;
			tokens.push_back(token);
		}
		else
		{
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
