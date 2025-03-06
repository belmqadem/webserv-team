#include "../../includes/webserv.hpp"
#include "../../includes/RequestParser.hpp"

#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

class CGIHandler {
private:
    std::string scriptPath;
    char *envp[5];
    std::string path;

public:
    CGIHandler();
    CGIHandler(std::string path);
    CGIHandler(std::string pathS, std::string pathE);
    
    void setupEnvironment();
    void executeCGI(RequestParser &request);
    char **setupExecveArgs(const std::string& path);
    void execute();
};

#endif
// #include <iostream>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <cstdlib>
// #include <vector>

// void executeCGI(const std::string& scriptPath, const std::string& method, const std::string& body, const std::string& queryString) {
//     int pipefd[2]; // Create a pipe (pipefd[0] = read, pipefd[1] = write)
//     pipe(pipefd);

//     pid_t pid = fork();
    
//     if (pid == 0) {  // Child process
//         close(pipefd[0]); // Close read end, we only write here
        
//         // Redirect stdout and stderr to the pipe
//         dup2(pipefd[1], STDOUT_FILENO);
//         dup2(pipefd[1], STDERR_FILENO);
//         close(pipefd[1]); // Close unused write end

//         // Set environment variables
//         setenv("REQUEST_METHOD", method.c_str(), 1);
//         setenv("QUERY_STRING", queryString.c_str(), 1);
        
//         if (method == "POST") {
//             int postPipe[2];
//             pipe(postPipe);
//             write(postPipe[1], body.c_str(), body.size());
//             close(postPipe[1]);
//             dup2(postPipe[0], STDIN_FILENO);
//             close(postPipe[0]);
//         }

//         char *args[] = {nullptr};  // No additional args
//         execve(scriptPath.c_str(), args, environ);
//         exit(1);  // If execve fails
//     } 
//     else {  // Parent process
//         close(pipefd[1]); // Close write end, we only read here

//         char buffer[1024];
//         std::string cgiOutput;
//         ssize_t bytesRead;
        
//         while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
//             buffer[bytesRead] = '\0';
//             cgiOutput += buffer;
//         }

//         close(pipefd[0]); // Close read end
//         waitpid(pid, nullptr, 0); // Wait for child process

//         // Process the CGI output (headers & body)
//         std::cout << "CGI Output:\n" << cgiOutput << std::endl;
//     }
// }

// int main() {
//     std::string scriptPath = "/cgi-bin/script.py";
//     std::string method = "GET"; 
//     std::string body = "";  
//     std::string queryString = "id=123&name=admin"; 

//     executeCGI(scriptPath, method, body, queryString);
//     return 0;
// }

