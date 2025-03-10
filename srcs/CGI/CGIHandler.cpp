#include "../../includes/CGIHandler.hpp"

#define ROOT_DIRECTORY "www/html"

CGIHandler::CGIHandler(RequestParser &request, const std::string &php_cgi_path)
{
	Logger &logger = Logger::getInstance();
	scriptPath = ROOT_DIRECTORY + request.get_request_uri();
	method = request.get_http_method();
	queryString = request.get_query_string();
	body = request.get_body();
	headers = request.get_headers();

	size_t dotPos = scriptPath.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		std::string extension = scriptPath.substr(dotPos);
		if (extension == ".php")
		{
			interpreter = php_cgi_path;
		}
	}
	if (interpreter.empty())
	{
		logger.error("No CGI interpreter found");
		throw std::runtime_error("500 Internal Server Error: No CGI interpreter found");
	}
	logger.info("CGIHandler initialized with script: " + scriptPath);
}
std::string CGIHandler::executeCGI() {
    Logger &logger = Logger::getInstance();
    int output_pipe[2], input_pipe[2];
    if (pipe(output_pipe) == -1 || pipe(input_pipe) == -1) {
        throw std::runtime_error("500 Internal Server Error: Pipe creation failed");
    }
    pid_t pid = fork();
    if (pid == -1) {
        logger.error("Fork failed");
        throw std::runtime_error("500 Internal Server Error: Fork failed");
    }
    if (pid == 0) {
        close(output_pipe[0]);
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[1]);
        if (method == "POST") {
            close(input_pipe[1]);
            dup2(input_pipe[0], STDIN_FILENO);
            close(input_pipe[0]);
        }
        std::vector<std::string> args;
        args.push_back(interpreter);
        args.push_back(scriptPath);
        char *argv[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i)
            argv[i] = const_cast<char *>(args[i].c_str());
        argv[args.size()] = NULL;
        std::vector<std::string> env;
        env.push_back("REQUEST_METHOD=" + method);
        env.push_back("QUERY_STRING=" + queryString);
        env.push_back("CONTENT_LENGTH=" + std::to_string(body.length()));
        env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
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
        exit(1);
    } else {
        close(output_pipe[1]);
        close(input_pipe[0]);
        if (method == "POST") {
            write(input_pipe[1], body.c_str(), body.length());
            close(input_pipe[1]);
        }
        char buffer[1024];
        std::string cgi_output;
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            cgi_output += buffer;
        }
        close(output_pipe[0]);
        waitpid(pid, NULL, 0);
        return cgi_output;
    }
}
// std::string CGIHandler::executeCGI() {
//     Logger &logger = Logger::getInstance();
//     int output_pipe[2], input_pipe[2];
//     if (pipe(output_pipe) == -1 || pipe(input_pipe) == -1) {
//         throw std::runtime_error("500 Internal Server Error: Pipe creation failed");
//     }
//     pid_t pid = fork();
//     if (pid == -1) {
//         logger.error("Fork failed");
//         throw std::runtime_error("500 Internal Server Error: Fork failed");
//     }
//     if (pid == 0) {
//         close(output_pipe[0]);
//         dup2(output_pipe[1], STDOUT_FILENO);
//         close(output_pipe[1]);
//         if (method == "POST") {
//             close(input_pipe[1]);
//             dup2(input_pipe[0], STDIN_FILENO);
//             close(input_pipe[0]);
//         }
//         std::vector<std::string> args;
//         args.push_back(interpreter);
//         args.push_back(scriptPath);
//         char *argv[args.size() + 1];
//         for (size_t i = 0; i < args.size(); ++i)
//             argv[i] = const_cast<char *>(args[i].c_str());
//         argv[args.size()] = NULL;
//         std::vector<std::string> env;
//         env.push_back("REQUEST_METHOD=" + method);
//         env.push_back("QUERY_STRING=" + queryString);
//         env.push_back("CONTENT_LENGTH=" + std::to_string(body.length()));
//         env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
//         env.push_back("REDIRECT_STATUS=200");
//         env.push_back("SCRIPT_FILENAME=" + scriptPath);
//         env.push_back("SCRIPT_NAME=" + scriptPath);
//         env.push_back("DOCUMENT_ROOT=" + std::string(ROOT_DIRECTORY));
//         env.push_back("PHP_SELF=" + scriptPath);
//         env.push_back("PATH_TRANSLATED=" + scriptPath);
//         char *envp[env.size() + 1];
//         for (size_t i = 0; i < env.size(); ++i)
//             envp[i] = const_cast<char *>(env[i].c_str());
//         envp[env.size()] = NULL;
//         execve(argv[0], argv, envp);
//         logger.error("execve failed");
//         exit(1);
//     } else {
//         close(output_pipe[1]);
//         close(input_pipe[0]);
//         if (method == "POST") {
//             write(input_pipe[1], body.c_str(), body.length());
//             close(input_pipe[1]);
//         }
//         char buffer[1024];
//         std::string cgi_output;
//         ssize_t bytesRead;
//         while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
//             buffer[bytesRead] = '\0';
//             cgi_output += buffer;
//         }
//         close(output_pipe[0]);
//         waitpid(pid, NULL, 0);
//         return cgi_output;
//     }
// }
// std::string CGIHandler::executeCGI()
// {
// 	Logger &logger = Logger::getInstance();
// 	int output_pipe[2], input_pipe[2];
// 	if (pipe(output_pipe) == -1 || pipe(input_pipe) == -1)
// 	{
// 		// logger.error("Pipe creation failed");
// 		throw std::runtime_error("500 Internal Server Error: Pipe creation failed");
// 	}
// 	pid_t pid = fork();
// 	if (pid == -1)
// 	{
// 		logger.error("Fork failed");
// 		throw std::runtime_error("500 Internal Server Error: Fork failed");
// 	}
// 	if (pid == 0)
// 	{
// 		// logger.info("Executing CGI script: " + scriptPath);
// 		// logger.info("Using interpreter: " + interpreter);
// 		close(output_pipe[0]);
// 		dup2(output_pipe[1], STDOUT_FILENO);
// 		close(output_pipe[1]);
// 		if (method == "POST")
// 		{
// 			close(input_pipe[1]);
// 			dup2(input_pipe[0], STDIN_FILENO);
// 			close(input_pipe[0]);
// 		}
// 		std::vector<std::string> args;
// 		args.push_back(interpreter);
// 		args.push_back(scriptPath);
//         // std::cout <<  scriptPath <<std::endl; exit(0);
// 		char *argv[args.size() + 1];
// 		for (size_t i = 0; i < args.size(); ++i)
// 			argv[i] = const_cast<char *>(args[i].c_str());
// 		argv[args.size()] = NULL;
//         std::vector<std::string> env;
//         env.push_back("REQUEST_METHOD=" + method);
//         env.push_back("QUERY_STRING=" + queryString);
// 		// std::cout << queryString <<std::endl;exit(0);
//         env.push_back("CONTENT_LENGTH=" + std::to_string(body.length()));
//         env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
//         env.push_back("REDIRECT_STATUS=200"); 
//         // std::cout << queryString <<std::endl;
//         char *envp[env.size() + 1];
// 		for (size_t i = 0; i < env.size(); ++i)
// 			envp[i] = const_cast<char *>(env[i].c_str());
// 		envp[env.size()] = NULL;
// 		execve(argv[0], argv, envp);
// 		logger.error("execve failed");
// 		exit(1);
// 	}
// 	else
// 	{
// 		close(output_pipe[1]);
// 		close(input_pipe[0]);
// 		if (method == "POST")
// 		{
// 			write(input_pipe[1], body.c_str(), body.length());
// 			close(input_pipe[1]);
// 		}
// 		char buffer[1024];
// 		std::string cgi_output;
// 		ssize_t bytesRead;
// 		while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
// 		{
// 			buffer[bytesRead] = '\0';
// 			cgi_output += buffer;
// 		}
// 		close(output_pipe[0]);
// 		waitpid(pid, NULL, 0);
// 		return cgi_output;
// 	}
// }