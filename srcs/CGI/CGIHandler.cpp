#include "CGIHandler.hpp"

#define ROOT_DIRECTORY "www/html"

CGIHandler::CGIHandler(RequestParser &request, const std::string &_path)
{
	Logger &logger = Logger::getInstance();
	scriptPath = ROOT_DIRECTORY + request.get_request_uri();
	method = request.get_http_method();
	queryString = request.get_query_string();
	body = std::string(request.get_body().begin(), request.get_body().end());
	headers = request.get_headers();

	size_t dotPos = scriptPath.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		std::string extension = scriptPath.substr(dotPos);
		if (extension == ".php")
		{
			interpreter = _path;
		}
		else if (extension == ".py")
		{
			interpreter = "usr/bin/python3";
		}
	}
	if (interpreter.empty())
	{
		logger.error("No CGI interpreter found");
		throw std::runtime_error("500 Internal Server Error: No CGI interpreter found");
	}
	logger.info("CGIHandler initialized with script: " + scriptPath);
}
std::string CGIHandler::executeCGI()
{
	Logger &logger = Logger::getInstance();
	int cgi_socket[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, cgi_socket) == -1)
	{
		throw std::runtime_error("500 Internal Server Error: Socketpair creation failed");
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		logger.error("Fork failed");
		throw std::runtime_error("500 Internal Server Error: Fork failed");
	}
	if (pid == 0)
	{
		close(cgi_socket[0]);
		dup2(cgi_socket[1], STDOUT_FILENO);
		dup2(cgi_socket[1], STDIN_FILENO);
		close(cgi_socket[1]);
		std::ostringstream oss;
		oss << body.length();
		std::vector<std::string> args;
		args.push_back(interpreter);
		args.push_back(scriptPath);
		char *argv[args.size() + 1];
		for (size_t i = 0; i < args.size(); ++i)
			argv[i] = const_cast<char *>(args[i].c_str());
		argv[args.size()] = NULL;
		std::vector<std::string> env;
		env.push_back("GATEWAY_INTERFACE=CGI/1.1");
		env.push_back("SERVER_PROTOCOL=HTTP/1.1");
		env.push_back("SERVER_SOFTWARE=CustomServer/1.0");
		env.push_back("REQUEST_METHOD=" + method);
		env.push_back("QUERY_STRING=" + queryString);
		env.push_back("CONTENT_LENGTH=" + oss.str());
		env.push_back("CONTENT_TYPE=" + (headers.count("Content-Type") ? headers["Content-Type"] : "application/x-www-form-urlencoded"));
		env.push_back("REDIRECT_STATUS=200");
		env.push_back("SCRIPT_FILENAME=" + scriptPath);
		env.push_back("SCRIPT_NAME=" + scriptPath);
		env.push_back("DOCUMENT_ROOT=" + std::string(ROOT_DIRECTORY));
		env.push_back("PHP_SELF=" + scriptPath);
		env.push_back("PATH_TRANSLATED=" + scriptPath);
		char *envp[env.size() + 1];
		for (size_t i = 0; i < env.size(); ++i)
			envp[i] = const_cast<char *>(env[i].c_str());
		envp[env.size()] = NULL;
		execve(argv[0], argv, envp);
		logger.error("execve failed");
		exit(127);
	}
	else
	{
		close(cgi_socket[1]);
		if (method == "POST")
		{
			write(cgi_socket[0], body.c_str(), body.length());
			logger.info("CGI POST Body: " + body);
		}
		char buffer[1024];
		std::string cgi_output;
		ssize_t bytesRead;
		while ((bytesRead = read(cgi_socket[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytesRead] = '\0';
			cgi_output += buffer;
		}
		logger.info("CGI Output: " + cgi_output);
		close(cgi_socket[0]);
		int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) // Check child exit status
        {
            throw std::runtime_error("500 Internal Server Error: CGI execution failed");
        }
		return cgi_output;
	}
}
