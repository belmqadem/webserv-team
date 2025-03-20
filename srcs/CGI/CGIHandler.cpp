#include "CGIHandler.hpp"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>

CGIHandler::CGIHandler(RequestParser& request, const ServerConfig& serverConfig, const Location& locationConfig)
    : _request(request), _serverConfig(serverConfig), _locationConfig(locationConfig) { 
    std::cout << "hello from constructor" << std::endl;
    }

CGIHandler::~CGIHandler() {}

void CGIHandler::executeCGI() {
    // Setup environment variables
    _env["REQUEST_METHOD"] = _request.get_http_method();
    _env["QUERY_STRING"] = _request.get_query_string();
    std::cout << "hello from execve" << std::endl;
    
    std::ostringstream contentLengthStream;
    contentLengthStream << _request.get_body().size();
    _env["CONTENT_LENGTH"] = contentLengthStream.str();
    
    _env["CONTENT_TYPE"] = _request.get_header_value("content-type");
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["SERVER_NAME"] = _serverConfig.serverNames[0];
    
    std::ostringstream portStream;
    portStream << _serverConfig.port;
    _env["SERVER_PORT"] = portStream.str();
    
    _env["SCRIPT_NAME"] = _request.get_request_uri();
    _env["PATH_INFO"] = _request.get_request_uri();
    _env["PATH_TRANSLATED"] = _locationConfig.cgiWorkingDirectory + _request.get_request_uri();
    
    // Setup CGI arguments
    std::string scriptPath = _locationConfig.cgiWorkingDirectory + _request.get_request_uri();
    _args.clear();
    _args.push_back(scriptPath);
    if (!_request.get_query_string().empty()) {
        _args.push_back(_request.get_query_string());
    }
    
    int sockfd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1) {
        throw std::runtime_error("500 Internal Server Error: Socketpair creation failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(sockfd[0]);
        close(sockfd[1]);
        throw std::runtime_error("500 Internal Server Error: Fork failed");
    }

    if (pid == 0) {
        close(sockfd[0]);
        dup2(sockfd[1], STDIN_FILENO);
        dup2(sockfd[1], STDOUT_FILENO);
        close(sockfd[1]);

        std::vector<const char*> envp;
        for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); ++it) {
            std::string env_entry = it->first + "=" + it->second;
            envp.push_back(strdup(env_entry.c_str()));
        }
        envp.push_back(NULL);

        std::vector<const char*> argv;
        for (std::vector<std::string>::const_iterator it = _args.begin(); it != _args.end(); ++it) {
            argv.push_back(it->c_str());
        }
        argv.push_back(NULL);
        std::cerr << "hello from execve" << std::endl;
        execve(argv[0], const_cast<char* const*>(&argv[0]), const_cast<char* const*>(&envp[0]));
        exit(1);
    } else {
        close(sockfd[1]);
        
        if (_env["REQUEST_METHOD"] == "POST") {
            const std::vector<unsigned char>& body = _request.get_body();
            write(sockfd[0], &body[0], body.size());
        }
        
        char buffer[4096];
        ssize_t bytesRead;
        _cgiOutput.clear();
        while ((bytesRead = read(sockfd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            _cgiOutput.append(buffer, bytesRead);
        }
        close(sockfd[0]);
        waitpid(pid, NULL, 0);
    }
}


std::string CGIHandler::getOutput() const {
    return _cgiOutput;
}