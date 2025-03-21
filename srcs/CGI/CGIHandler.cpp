#include "CGIHandler.hpp"
#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>
#include <cstring>
#include <fcntl.h>
#include <iostream>

// CGIHandler::CGIHandler(const std::string& scriptPath, const std::map<std::string, std::string>& envVars)
//     : _scriptPath(scriptPath), _envVars(envVars) {}

// CGIHandler::~CGIHandler() {}

// std::string CGIHandler::execute(const std::string& method, const std::string& body) {
//     int pipeIn[2], pipeOut[2];

//     if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
//         throw std::runtime_error("Failed to create pipes.");
//     }

//     pid_t pid = fork();
//     if (pid < 0) { // Fork failed
//         close(pipeIn[0]);
//         close(pipeIn[1]);
//         close(pipeOut[0]);
//         close(pipeOut[1]);
//         throw std::runtime_error("Failed to fork process.");
//     }

//     if (pid == 0) { // Child process
//         close(pipeIn[1]);
//         close(pipeOut[0]);

//         // Redirect pipes to standard input and output
//         if (dup2(pipeIn[0], STDIN_FILENO) == -1 || dup2(pipeOut[1], STDOUT_FILENO) == -1) {
//             perror("dup2 failed");
//             exit(EXIT_FAILURE);
//         }

//         close(pipeIn[0]);
//         close(pipeOut[1]);

//         // Set up environment variables
//         char** env = createEnvArray();

//         // Configure arguments for execve
//         const char* interpreter = "/usr/bin/php-cgi"; // Adjust for your CGI script
//         const char* script = _scriptPath.c_str();
//         char* const argv[] = {const_cast<char*>(interpreter), const_cast<char*>(script), NULL};

//         // Execute the CGI script
//         execve(interpreter, argv, env);

//         // Handle execve failure
//         perror("execve failed");
//         cleanupEnvArray(env);
//         exit(EXIT_FAILURE);
//     } else { // Parent process
//         close(pipeIn[0]);
//         close(pipeOut[1]);

//         // Write the body if it's a POST request
//         if (method == "POST" && !body.empty()) {
//             if (write(pipeIn[1], body.c_str(), body.size()) == -1) {
//                 perror("write to pipe failed");
//             }
//         }
//         close(pipeIn[1]);

//         // Read CGI output
//         std::string output = readFromSocket(pipeOut[0]);
//         close(pipeOut[0]);

//         // Wait for the child process to finish
//         int status;
//         if (waitpid(pid, &status, 0) == -1) {
//             perror("waitpid failed");
//             throw std::runtime_error("Failed to wait for child process.");
//         }

//         if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
//             throw std::runtime_error("CGI script execution failed with non-zero exit code.");
//         }

//         return output;
//     }
// }

// char** CGIHandler::createEnvArray() const {
//     char** envArray = new char*[_envVars.size() + 1];
//     size_t i = 0;
//     for (std::map<std::string, std::string>::const_iterator it = _envVars.begin(); it != _envVars.end(); ++it, ++i) {
//         std::string envEntry = it->first + "=" + it->second;
//         envArray[i] = strdup(envEntry.c_str());
//     }
//     envArray[i] = NULL;
//     return envArray;
// }

// void CGIHandler::cleanupEnvArray(char** envArray) const {
//     for (size_t i = 0; envArray[i] != NULL; ++i) {
//         free(envArray[i]);
//     }
//     delete[] envArray;
// }

// std::string CGIHandler::readFromSocket(int socketFd) {
//     std::ostringstream output;
//     char buffer[1024];
//     ssize_t bytesRead;

//     while ((bytesRead = read(socketFd, buffer, sizeof(buffer))) > 0) {
//         output.write(buffer, static_cast<std::streamsize>(bytesRead));
//     }

//     if (bytesRead < 0) {
//         perror("read from pipe failed");
//         throw std::runtime_error("Error reading from pipe.");
//     }

//     return output.str();
// }
// #include "../../includes/CGIHandler.hpp"

#define ROOT_DIRECTORY "www/html"

CGIHandler::CGIHandler(RequestParser &request, const std::string &php_cgi_path)
{
	Logger &logger = Logger::getInstance();
	scriptPath = ROOT_DIRECTORY + request.get_request_uri();
	method = request.get_http_method();
	queryString = request.get_query_string();
	body = std::string(request.get_body().begin(),request.get_body().end() );
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
        std::ostringstream oss;
        oss << (body.length());
        //     return oss.str();
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
        env.push_back("CONTENT_LENGTH=" + oss.str());
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
        std::cerr << argv[0] << "\n" << argv[1] << std::endl;
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

// int main() {
//     try {
//         // Define the path to your CGI script
//         std::string scriptPath = "www/html/test.php"; // Update this to your actual script path

//         // Set up environment variables
//         std::map<std::string, std::string> envVars;
//         envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
//         envVars["SERVER_PROTOCOL"] = "HTTP/1.1";
//         envVars["SERVER_NAME"] = "localhost";
//         envVars["SERVER_PORT"] = "8080";
//         envVars["REQUEST_METHOD"] = "GET";
//         envVars["SCRIPT_NAME"] = "/cgi-bin/test.php";
//         envVars["SCRIPT_FILENAME"] = scriptPath;
//         envVars["REMOTE_ADDR"] = "127.0.0.1";
//         envVars["REDIRECT_STATUS"] = "200";

//         // Initialize CGIHandler
//         CGIHandler cgiHandler(scriptPath, envVars);

//         // Execute the CGI script (GET request, no body)
//         std::string method = "GET";
//         std::string body = ""; // Empty body for GET

//         std::cout << "Executing CGI script..." << std::endl;
//         std::string output = cgiHandler.execute(method, body);

//         // Output the result
//         std::cout << "CGI Output:\n" << output << std::endl;

//     } catch (const std::exception& ex) {
//         // Catch and display any errors
//         std::cerr << "Error: " << ex.what() << std::endl;
//         return EXIT_FAILURE;
//     }
// }