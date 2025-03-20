#include "RequestParser.hpp"
#include "IEvenetListeners.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

class CGIHandler : public IEvenetListeners {
public:
    CGIHandler(RequestParser& request, const ServerConfig& serverConfig, const Location& locationConfig);
    virtual ~CGIHandler();

    void executeCGI();
    void setupEnvironment();
    void setupArgs();
    void executeScript();
    std::string getOutput() const;

    virtual void onEvent(int fd, epoll_event ev);
    virtual void terminate();

private:
    RequestParser& _request;
    const ServerConfig& _serverConfig;
    const Location& _locationConfig;
    std::map<std::string, std::string> _env;
    std::vector<std::string> _args;
    std::string _cgiOutput;
};

CGIHandler::CGIHandler(RequestParser& request, const ServerConfig& serverConfig, const Location& locationConfig)
    : _request(request), _serverConfig(serverConfig), _locationConfig(locationConfig) {}

CGIHandler::~CGIHandler() {}

void CGIHandler::setupEnvironment() {
    _env["REQUEST_METHOD"] = _request.get_http_method();
    _env["QUERY_STRING"] = _request.get_query_string();

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
    _env["PATH_TRANSLATED"] = _locationConfig.root + _request.get_request_uri();
}

void CGIHandler::setupArgs() {
    std::string scriptPath = _locationConfig.cgiWorkingDirectory + _request.get_request_uri();
    _args.push_back(scriptPath);
}

void CGIHandler::executeScript() {
    int output_pipe[2];
    if (pipe(output_pipe) == -1) {
        throw std::runtime_error("500 Internal Server Error: Pipe creation failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("500 Internal Server Error: Fork failed");
    }

    if (pid == 0) {
        close(output_pipe[0]);
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[1]);

        std::vector<const char*> envp;
        for (const auto& it : _env) {
            std::string env_entry = it.first + "=" + it.second;
            envp.push_back(strdup(env_entry.c_str()));
        }
        envp.push_back(NULL);

        std::vector<const char*> argv;
        for (const auto& it : _args) {
            argv.push_back(strdup(it.c_str()));
        }
        argv.push_back(NULL);

        execve(argv[0], const_cast<char* const*>(&argv[0]), const_cast<char* const*>(&envp[0]));
        exit(1);
    } else {
        close(output_pipe[1]);
        _cgiOutput.clear();
        char buffer[4096];
        ssize_t bytesRead;

        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            _cgiOutput += buffer;
        }
        close(output_pipe[0]);
        waitpid(pid, NULL, 0);
    }
}

void CGIHandler::executeCGI() {
    setupEnvironment();
    setupArgs();
    executeScript();
}

std::string CGIHandler::getOutput() const {
    return _cgiOutput;
}

void CGIHandler::onEvent(int fd, epoll_event ev) {}

void CGIHandler::terminate() {
    _cgiOutput.clear();
    _env.clear();
    _args.clear();
}
