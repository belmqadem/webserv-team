#include "CGIHandler.hpp"

CGIHandler::CGIHandler(std::string path) : scriptPath(path) , path("usr/bin/python3") 
{
    envp[0] = (char *)"REQUEST_METHOD=GET";
    envp[1] = (char *)"QUERY_STRING=name=hamid&age=21";
    envp[2] = (char *)"CONTENT_TYPE=text/plain";
    envp[3] = (char *)"CONTENT_LENGTH=0";
    envp[4] = NULL;
};
CGIHandler::CGIHandler(): scriptPath("") , path("usr/bin/python3")
{}
CGIHandler::CGIHandler(std::string pathS, std::string pathE): scriptPath(pathS) , path(pathE)  {} 



void CGIHandler::executeCGI(RequestParser &request) {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid = fork();
    
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        setenv("REQUEST_METHOD", ((request.get_http_method()).c_str()), 1);
        setenv("QUERY_STRING", ((request.get_query_string()).c_str()), 1);
        if (request.get_http_method() == "POST") {
            int postPipe[2];
            pipe(postPipe);
            write(postPipe[1], request.get_body().c_str(), request.get_body().size());
            close(postPipe[1]);
            dup2(postPipe[0], STDIN_FILENO);
            close(postPipe[0]);
        }
        char *args[] = {nullptr};
        execve(request.get_request_uri().c_str(), args, environ);
        exit(1);
    } 
    else {
        close(pipefd[1]);

        char buffer[1024];
        std::string cgiOutput;
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            cgiOutput += buffer;
        }
        close(pipefd[0]);
        waitpid(pid, nullptr, 0);
        std::cout << "CGI Output:\n" << cgiOutput << std::endl;
    }
}

// int main()
// {
//     CGIHandler a("a.py");
//     a.execute();
// }
