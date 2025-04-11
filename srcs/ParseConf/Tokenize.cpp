#include "Tokenize.hpp"
#include "webserv.hpp"

bool is_size_multiplier(char c)
{
	const std::string size_muls = "kmg";
	return size_muls.find(std::tolower(c)) != std::string::npos;
}

void SanitizeInput(std::string &input)
{
	const std::string delim = ";{}";
	for (size_t i = 0; i < input.length(); i++)
	{
		char c = input[i];

		if (delim.find(c) != std::string::npos)
		{
			input.insert(i, " ");
			input.insert(i + 2, " ");
			i += 2;
		}
		else if (is_size_multiplier(c) && i > 0 && std::isdigit(input[i - 1]))
		{
			input[i] = std::tolower(c);
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
	std::istringstream stream(input);
	std::string word;

	while (stream >> word)
	{
		// ignore comments
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
		else if (word == "upload_store")
		{
			token.type = UPLOAD_STORE;
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
		else if (word == "return")
		{
			token.type = RETURN;
			token.value = word;
			tokens.push_back(token);
		}
		else if (word.length() == 1 && is_size_multiplier(word[0]))
		{
			token.type = CLIENT_MAX_BODY_SIZE_MUL;
			token.value = word;
			tokens.push_back(token);
		}
		else if (Utils::string_to_size_t(word, token.nums))
		{
			if (token.nums < 0)
				throw std::runtime_error("Negative values is forbiden");
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
