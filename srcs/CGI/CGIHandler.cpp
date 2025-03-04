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


char** CGIHandler::setupExecveArgs(const std::string& path)
{
    char **args;
    args[0] = (char *)(path.c_str());
    args[1] = (char *)scriptPath.c_str();
    args[2] = NULL;
    return args;
}
void CGIHandler::execute() {
    int pipe_out[2];
    pipe(pipe_out);
    pid_t pid = fork();
    if (pid == 0) {
        close(pipe_out[0]);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_out[1]);
        char **argv =  setupExecveArgs(path);
        if(scriptPath.empty())
            throw std::runtime_error("No path found");
        execve(argv[0], argv, envp);
        exit(1);
    } else { 
        close(pipe_out[1]);
        char buffer[1024];
        int bytes = read(pipe_out[0], buffer, sizeof(buffer) - 1);
        buffer[bytes] = '\0';
        std::cout << "CGI Output:\n" << buffer << std::endl;
        close(pipe_out[0]);
        waitpid(pid, NULL, 0);
    }
}
int main()
{
    CGIHandler a("a.py");
    a.execute();
}
