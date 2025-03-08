#include "CGIHandler.hpp"


CGIHandler::CGIHandler(RequestParser &request, const std::string &php_cgi_path)
    {
        scriptPath = request.get_request_uri();
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
            throw std::runtime_error("500 Internal Server Error: No CGI interpreter found");

    }


std::string CGIHandler::executeCGI()
{
    int output_pipe[2], input_pipe[2];

    if (pipe(output_pipe) == -1 || pipe(input_pipe) == -1)
        throw std::runtime_error("500 Internal Server Error: Pipe creation failed");

    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error("500 Internal Server Error: Fork failed");

        if (pid == 0)
        {
            std::cout << "Executing CGI script: " << scriptPath << std::endl;
            std::cout << "Using interpreter: " << interpreter << std::endl;
        
            close(output_pipe[0]);
            dup2(output_pipe[1], STDOUT_FILENO);
            close(output_pipe[1]);
        
            if (method == "POST")
            {
                close(input_pipe[1]);
                dup2(input_pipe[0], STDIN_FILENO);
                close(input_pipe[0]);
            }
        
            std::vector<std::string> args;
            args.push_back(interpreter);
            args.push_back(scriptPath);
            args.push_back(queryString);
        
            char *argv[args.size() + 1];
            for (size_t i = 0; i < args.size(); ++i)
                argv[i] = const_cast<char *>(args[i].c_str());
            argv[args.size()] = NULL;
        
            std::vector<std::string> env;
            env.push_back("REQUEST_METHOD=" + method);
            env.push_back("QUERY_STRING=" + queryString);
            env.push_back("CONTENT_LENGTH=" + std::to_string(body.length()));
            env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
        
            char *envp[env.size() + 1];
            for (size_t i = 0; i < env.size(); ++i)
                envp[i] = const_cast<char *>(env[i].c_str());
            envp[env.size()] = NULL;
        
            std::cout << "Executing execve with args:" << std::endl;
            for (size_t i = 0; i < args.size(); ++i)
            {
                std::cout << args[i] << std::endl;
            }
        
            execve(argv[0], argv, envp);
            perror("execve failed");
            exit(1);
        }
        
    else
    {
        close(output_pipe[1]);
        close(input_pipe[0]);

        if (method == "POST")
        {
            write(input_pipe[1], body.c_str(), body.length());
            close(input_pipe[1]);
        }

        char buffer[1024];
        std::string cgi_output;
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            cgi_output += buffer;
        
            std::cout << "CGI Output: " << buffer << std::endl;
        }
        
        close(output_pipe[0]);
        waitpid(pid, NULL, 0);
        
        std::cout << "Final CGI Response: " << cgi_output << std::endl;
        

        return cgi_output;
    }
}






    


